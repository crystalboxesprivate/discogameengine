#include <behavior/physics.h>
#include <core/math/math.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <game/rigid_body_component.h>
#include <game/camera_component.h>
#include <glm/gtx/string_cast.hpp>

namespace behavior {
using namespace glm;
#pragma region Physics Bridge
struct sRigidBodyDef {
  sRigidBodyDef()
      : Mass(0.0f)
      , Position(0.0f, 0.0f, 0.0f)
      , Velocity(0.0f, 0.0f, 0.0f)
      , Orientation(0.0f, 0.0f, 0.0f)
      , AngularVelocity(0.0f, 0.0f, 0.0f)
      , GameObjectName("undefined")
      , isPlayer(false) {
  }
  float Mass;
  vec3 Position;
  vec3 Velocity;
  bool isPlayer;
  std::string GameObjectName;
  vec3 Orientation; // Euler Angles
  vec3 AngularVelocity;
  quat quatOrientation = quat(vec3(0.0f, 0.0f, 0.0f));
};

namespace nPhysics {
enum eShapeType {
  SHAPE_TYPE_PLANE = 0,
  SHAPE_TYPE_SPHERE = 1,
  SHAPE_TYPE_CYLINDER = 2,
  SHAPE_TYPE_CAPSULE = 3,
  SHAPE_TYPE_BOX = 4
};
}
namespace nConvert {
inline vec3 ToSimple(const btVector3 &vecIn) {
  return vec3(vecIn.x(), vecIn.y(), vecIn.z());
}
inline void ToSimple(const btVector3 vecIn, vec3 &vecOut) {
  vecOut.x = vecOut.x;
  vecOut.y = vecOut.y;
  vecOut.z = vecOut.z;
}
inline btVector3 ToBullet(const vec3 &vecIn) {
  return btVector3(vecIn.x, vecIn.y, vecIn.z);
}
inline void ToBullet(const vec3 &vecIn, btVector3 &vecOut) {
  vecOut.setValue(vecIn.x, vecIn.y, vecIn.z);
}

inline mat4 ToSimple(const btTransform &transformIn) {
  mat4 mat;
  transformIn.getOpenGLMatrix(&mat[0][0]);
  return mat;
}
inline void ToSimple(const btTransform &transform, mat4 matOut) {

  transform.getOpenGLMatrix(&matOut[0][0]);
}

inline quat ToSimple(const btQuaternion &quatIn) {
  quat glm_quat(quatIn.getW(), quatIn.getX(), quatIn.getY(), quatIn.getZ());
  return glm_quat;
}

inline btQuaternion ToBullet(const quat &quatIn) {
  btQuaternion quatBT;
  quatBT = btQuaternion(quatIn.x, quatIn.y, quatIn.z, quatIn.w);
  return quatBT;
}
} // namespace nConvert

struct iShape {
  virtual ~iShape(){};

  inline nPhysics::eShapeType GetShapeType() const {
    return mShapeType;
  }
  // virtual bool GetAABB(const mat4& transform, vec3& minBoundsOut, vec3& maxBoundsOut) { return false;
  // } /*=0*/
  virtual bool GetSphereRadius(float &radiusOut) {
    return false;
  }
  virtual bool GetPlaneConstant(float &planeConstantOut) {
    return false;
  }
  virtual bool GetPlaneNormal(vec3 &planeNormalOut) {
    return false;
  }
  // Cylinder
  virtual float GetCylinderRadius() {
    return 0;
  }
  virtual int GetCylinderAxis() {
    return 0;
  }
  virtual float GetCylinderHeight() {
    return 0.0f;
  }
  // Capsule
  virtual float GetCapsuleRadius() {
    return 0.0f;
  }
  virtual float GetCapsuleHeight() {
    return 0.0f;
  }
  virtual int GetCapsuleAxis() {
    return 0;
  }

  // Box
  virtual vec3 GetHalfExtents() {
    return vec3(0.f);
  }

protected:
  iShape(nPhysics::eShapeType shapeType)
      : mShapeType(shapeType) {
  }
  iShape(const iShape &other) {
  }
  iShape &operator=(const iShape &other) {
    return *this;
  }

private:
  nPhysics::eShapeType mShapeType;
};

#pragma region Shape

struct iSphereShape : public iShape {
  iSphereShape(float radius)
      : iShape(nPhysics::SHAPE_TYPE_SPHERE) {
    mRadius = radius;
    mBulletShape = new btSphereShape(btScalar(mRadius));
  }
  virtual ~iSphereShape() {
    delete mBulletShape;
  }
  virtual bool GetSphereRadius(float &radiusOut) {
    radiusOut = this->mRadius;
    return true;
  }
  inline btCollisionShape *GetBulletShape() {
    return mBulletShape;
  }

protected:
  float mRadius;
  iSphereShape(const iSphereShape &other)
      : iShape(other) {
  }
  iSphereShape &operator=(const iSphereShape &other) {
    return *this;
  }
  btCollisionShape *mBulletShape;
};

struct iPlaneShape : public iShape {
  iPlaneShape(const vec3 &normal, float planeConst)
      : iShape(nPhysics::SHAPE_TYPE_PLANE)
      , mNormal(normal)
      , mCnonst(planeConst) {
    this->mBulletShape = new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), planeConst);
  }

  virtual ~iPlaneShape() {
    delete mBulletShape;
  }

  virtual bool GetPlaneNormal(vec3 &planeNormalOut) {
    planeNormalOut = mNormal;
    return true;
  }

  virtual bool GetPlaneConstant(float &planeConstantOut) {
    planeConstantOut = mCnonst;
    return true;
  }

  inline btCollisionShape *GetBulletShape() {
    return mBulletShape;
  }

protected:
  vec3 mNormal;
  float mCnonst;
  iPlaneShape(const iPlaneShape &other)
      : iShape(other) {
  }
  iPlaneShape &operator=(const iPlaneShape &other) {
    return *this;
  }
  btCollisionShape *mBulletShape;
};

struct iCylinderShape : public iShape {
  iCylinderShape(const vec3 &halfExtents, int axis)
      : iShape(nPhysics::SHAPE_TYPE_CYLINDER)
      , mAxis(axis)
      , mRadius(halfExtents.x) {
    if (mAxis == 0) {
      // x
      this->mBulletShape = new btCylinderShapeX(nConvert::ToBullet(halfExtents));
    }
    if (mAxis == 1) {
      // y
      this->mBulletShape = new btCylinderShape(nConvert::ToBullet(halfExtents));
    } else {
      // z
      this->mBulletShape = new btCylinderShapeZ(nConvert::ToBullet(halfExtents));
    }
  }
  virtual ~iCylinderShape() {
  }
  virtual float GetCylinderRadius() {
    if (mAxis == 0) {
      // x
      return dynamic_cast<btCylinderShapeX *>(this->mBulletShape)->getRadius();
    }
    if (mAxis == 1) {
      // y
      return dynamic_cast<btCylinderShape *>(this->mBulletShape)->getRadius();
    } else {
      // z
      return dynamic_cast<btCylinderShapeZ *>(this->mBulletShape)->getRadius();
    }
  }
  virtual float GetCylinderHeight() {
    return 0.0f;
  }
  virtual int GetCylinderAxis() {
    return mAxis;
  }

  inline btCollisionShape *GetBulletShape() {
    return mBulletShape;
  }

protected:
  float mRadius;
  int mAxis;
  iCylinderShape(const iCylinderShape &other)
      : iShape(other) {
  }
  iCylinderShape &operator=(const iCylinderShape &other) {
    return *this;
  }
  btCollisionShape *mBulletShape;
};

struct iCapsuleShape : public iShape {
  iCapsuleShape(float height, float radius, int axis)
      : iShape(nPhysics::SHAPE_TYPE_CAPSULE)
      , mRadius(radius)
      , mAxis(axis)
      , mHeight(height) {
    {
      if (mAxis == 0) {
        // x
        this->mBulletShape = new btCapsuleShapeX(radius, height / 2);
      }
      if (mAxis == 1) {
        // y
        this->mBulletShape = new btCapsuleShape(radius, height / 2);
      } else {
        // z
        this->mBulletShape = new btCapsuleShapeZ(radius, height / 2);
      }
      // mBulletShape = shape;
    }
  }

  virtual ~iCapsuleShape() {
  }
  virtual float GetCapsuleRadius() {
    return this->mRadius;
  }
  virtual float GetCapsuleHeight() {
    return this->mHeight;
  }
  virtual int GetCapsuleAxis() {
    return this->mAxis;
  }
  inline btCollisionShape *GetBulletShape() {
    return mBulletShape;
  }

protected:
  float mRadius;
  int mAxis;
  float mHeight;
  iCapsuleShape(const iCapsuleShape &other)
      : iShape(other) {
  }
  iCapsuleShape &operator=(const iCapsuleShape &other) {
    return *this;
  }
  btCollisionShape *mBulletShape;
};

struct iBoxShape : public iShape {
  iBoxShape(const vec3 &halfExtents)
      : iShape(nPhysics::SHAPE_TYPE_BOX) {
    this->mBulletShape = new btBoxShape(nConvert::ToBullet(halfExtents));
  }

  virtual ~iBoxShape() {
  }

  inline vec3 GetHalfExtents() {
    return mHalfExtents;
  };
  inline btCollisionShape *GetBulletShape() {
    return mBulletShape;
  }

protected:
  vec3 mHalfExtents;
  iBoxShape(const iBoxShape &other)
      : iShape(other) {
  }
  iBoxShape &operator=(const iBoxShape &other) {
    return *this;
  }
  btCollisionShape *mBulletShape;
};
#pragma endregion

struct iRigidBody {
private:
  btDefaultMotionState *mMotionState;
  btRigidBody *mBody;
  iShape *mShape;
  bool bHasCollided;
  std::string mGameObjectName;
  float mMass;

public:
  iShape *iRigidBody::GetShape() {
    return mShape;
  }
  inline btRigidBody *GetBulletBody() {
    return mBody;
  }

  mat4 iRigidBody::GetTransform() {
    btTransform tranform;
    mMotionState->getWorldTransform(tranform);
    return nConvert::ToSimple(tranform);
  }

  vec3 iRigidBody::GetPosition() {

    btVector3 btVec = this->mBody->getCenterOfMassPosition();
    return nConvert::ToSimple(btVec);
  }

  vec3 iRigidBody::GetEulerRotation() {
    return vec3();
  }

  mat4 iRigidBody::GetMatRotation() {
    btQuaternion btQuat = this->mBody->getOrientation();

    quat quatRot = nConvert::ToSimple(btQuat);
    return toMat4(quatRot);
  }

  float iRigidBody::GetMass() {
    return this->mMass;
  }

  bool iRigidBody::GetCollision() {
    return this->bHasCollided;
  }

  vec3 iRigidBody::GetVelocity() {
    btVector3 vel = this->mBody->getLinearVelocity();
    return nConvert::ToSimple(vel);
  }

  vec3 iRigidBody::GetAccel() {
    return vec3();
  }

  vec3 iRigidBody::GetAngulatVelocity() {
    return nConvert::ToSimple(mBody->getAngularVelocity());
  }

  std::string iRigidBody::GetGOName() {
    return mGameObjectName;
  }

  void iRigidBody::SetPosition(vec3 position) {
    btQuaternion q(0, 0, 0);
    btTransform tr(q, nConvert::ToBullet(position));
    this->mBody->setWorldTransform(tr);
    this->mMotionState->setWorldTransform(tr);
  }

  void iRigidBody::SetEulerRotation(vec3 rotation) {
  }

  void iRigidBody::SetMatRotation(mat4 rotation) {
    // btMotionState* state = this->mBody->getMotionState();
    // btTransform startTransform;
    // state->getWorldTransform(startTransform);
    // startTransform.getRotation().m;
    // this->mBody->setMotionState()
  }

  void iRigidBody::SetMass(float mass) {
  }

  void iRigidBody::SetVelocity(vec3 velocity) {
    // this->mBody->activate();
    this->mBody->setLinearVelocity(nConvert::ToBullet(velocity));
  }
  void iRigidBody::SettAccel(vec3 accel) {
  }
  void iRigidBody::SetCollision(bool coll) {
    this->bHasCollided = coll;
  }

  iRigidBody::~iRigidBody() {
    mBody->setUserPointer(0);
    delete mBody;
    mBody = 0;
    if (mMotionState)
      delete mMotionState;
    mMotionState = 0;
  }
  iRigidBody(const sRigidBodyDef &def, iShape *shape)
      : mShape(shape) {
    mMotionState = nullptr;
    this->bHasCollided = false;
    this->mGameObjectName = def.GameObjectName;
    switch (shape->GetShapeType()) {
    case nPhysics::SHAPE_TYPE_PLANE: {
      btCollisionShape *colShape = dynamic_cast<iPlaneShape *>(shape)->GetBulletShape();
      btTransform startTransform;
      startTransform.setIdentity();
      mMass = def.Mass;
      btScalar mass(mMass);
      bool isDynamic = (mass != 0.f);
      btVector3 localInertia(0, 0, 0);

      if (isDynamic) {
        colShape->calculateLocalInertia(mass, localInertia);
      }

      btRigidBody::btRigidBodyConstructionInfo rbInfo(0, 0, colShape, localInertia);
      rbInfo.m_restitution = 0.9f;
      rbInfo.m_friction = 10.0f;
      mBody = new btRigidBody(rbInfo);

      mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
      mBody->setUserPointer(this);

      break;
    }
    case nPhysics::SHAPE_TYPE_SPHERE: {
      btCollisionShape *colShape = dynamic_cast<iSphereShape *>(shape)->GetBulletShape();
      btTransform startTransform;
      startTransform.setIdentity();

      mMass = def.Mass;
      btScalar mass(mMass);
      bool isDynamic = (mass != 0.f);
      btVector3 localInertia(0, 0, 0);

      if (isDynamic) {
        colShape->calculateLocalInertia(mass, localInertia);
      }
      startTransform.setOrigin(nConvert::ToBullet(def.Position));
      startTransform.setRotation(nConvert::ToBullet(def.quatOrientation));
      mMotionState = new btDefaultMotionState(startTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, mMotionState, colShape, localInertia);
      rbInfo.m_restitution = 0.9f;
      rbInfo.m_friction = 10.2f;
      mBody = new btRigidBody(rbInfo);
      mBody->setLinearVelocity(nConvert::ToBullet(def.Velocity));
      mBody->setAngularVelocity(nConvert::ToBullet(def.AngularVelocity));

      mBody->setSleepingThresholds(0.0f, 0.0f);

      mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
      mBody->setUserPointer(this);

      break;
    }

    case nPhysics::SHAPE_TYPE_CYLINDER: {
      btCollisionShape *colShape = dynamic_cast<iCylinderShape *>(shape)->GetBulletShape();
      btTransform startTransform;
      startTransform.setIdentity();

      mMass = def.Mass;
      btScalar mass(mMass);
      bool isDynamic = (mass != 0.f);
      btVector3 localInertia(0, 0, 0);

      if (isDynamic) {
        colShape->calculateLocalInertia(mass, localInertia);
      }
      startTransform.setOrigin(nConvert::ToBullet(def.Position));
      startTransform.setRotation(nConvert::ToBullet(def.quatOrientation));
      mMotionState = new btDefaultMotionState(startTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, mMotionState, colShape, localInertia);
      rbInfo.m_restitution = 0.9f;
      rbInfo.m_friction = 10.2f;
      mBody = new btRigidBody(rbInfo);
      mBody->setLinearVelocity(nConvert::ToBullet(def.Velocity));
      mBody->setAngularVelocity(nConvert::ToBullet(def.AngularVelocity));
      // mBody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
      mBody->setSleepingThresholds(0.0f, 0.0f);

      mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
      mBody->setUserPointer(this);

      break;
    }

    case nPhysics::SHAPE_TYPE_BOX: {
      btCollisionShape *colShape = dynamic_cast<iBoxShape *>(shape)->GetBulletShape();
      btTransform startTransform;
      startTransform.setIdentity();

      mMass = def.Mass;
      btScalar mass(mMass);
      bool isDynamic = (mass != 0.f);
      btVector3 localInertia(0, 0, 0);

      if (isDynamic) {
        colShape->calculateLocalInertia(mass, localInertia);
      }
      startTransform.setOrigin(nConvert::ToBullet(def.Position));
      startTransform.setRotation(nConvert::ToBullet(def.quatOrientation));
      mMotionState = new btDefaultMotionState(startTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, mMotionState, colShape, localInertia);
      rbInfo.m_restitution = 0.2f;
      rbInfo.m_friction = 10.2f;
      mBody = new btRigidBody(rbInfo);
      mBody->setLinearVelocity(nConvert::ToBullet(def.Velocity));
      mBody->setAngularVelocity(nConvert::ToBullet(def.AngularVelocity));
      // mBody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
      mBody->setSleepingThresholds(0.0f, 0.0f);

      mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
      mBody->setUserPointer(this);

      break;
    }

    case nPhysics::SHAPE_TYPE_CAPSULE: {
      btCollisionShape *colShape = dynamic_cast<iCapsuleShape *>(shape)->GetBulletShape();
      btTransform startTransform;
      startTransform.setIdentity();

      mMass = def.Mass;
      btScalar mass(mMass);
      bool isDynamic = (mass != 0.f);
      btVector3 localInertia(0, 0, 0);

      if (isDynamic) {
        colShape->calculateLocalInertia(mass, localInertia);
      }
      startTransform.setOrigin(nConvert::ToBullet(def.Position));
      startTransform.setRotation(nConvert::ToBullet(def.quatOrientation));

      mMotionState = new btDefaultMotionState(startTransform);
      if (!def.isPlayer) {
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, mMotionState, colShape, localInertia);
        rbInfo.m_restitution = 0.9f;
        rbInfo.m_friction = 10.2f;
        mBody = new btRigidBody(rbInfo);
        mBody->setLinearVelocity(nConvert::ToBullet(def.Velocity));
        mBody->setAngularVelocity(nConvert::ToBullet(def.AngularVelocity));
        mBody->setSleepingThresholds(0.0f, 0.0f);

      } else {
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, mMotionState, colShape, localInertia);
        rbInfo.m_restitution = 0.0f;
        rbInfo.m_friction = 0;
        mBody = new btRigidBody(rbInfo);
        mBody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
        // mBody->setLinearFactor(btVector3(0, 1, 0));
        // mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        mBody->setSleepingThresholds(0.0f, 0.0f);
      }

      mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
      mBody->setUserPointer(this);

      break;
    }
    default: { break; }
    }
  }
};
#pragma region Constraint
enum eConstraintType {
  CONSTRAINT_TYPE_POINT_TO_POINT,
  CONSTRAINT_TYPE_HINGE,
};
struct iConstraint {
  virtual ~iConstraint() {
  }
  iConstraint(eConstraintType constraintType)
      : mConstraintType(constraintType) {
  }
  iConstraint() {
  }
  iConstraint(const iConstraint &other) {
  }
  iConstraint &operator=(const iConstraint &other) {
    return *this;
  }
  inline eConstraintType GetConstraintType() const {
    return mConstraintType;
  }

private:
  eConstraintType mConstraintType;
};
class cBulletPoinToPointConstraint : public iConstraint {
public:
  cBulletPoinToPointConstraint(iRigidBody *rb, const btVector3 &pivot)
      : iConstraint(CONSTRAINT_TYPE_POINT_TO_POINT) {
    mConstraint = new btPoint2PointConstraint(*rb->GetBulletBody(), pivot);
  }
  cBulletPoinToPointConstraint(iRigidBody *rbA, iRigidBody *rbB, const btVector3 &pivotInA, const btVector3 &pivotInB)
      : iConstraint(CONSTRAINT_TYPE_POINT_TO_POINT) {
    mConstraint = new btPoint2PointConstraint(*rbA->GetBulletBody(), *rbB->GetBulletBody(), pivotInA, pivotInB);
  }
  virtual ~cBulletPoinToPointConstraint() {
  }

  virtual btTypedConstraint *GetTypedConstraint() {
    return mConstraint;
  }

private:
  btPoint2PointConstraint *mConstraint;
  cBulletPoinToPointConstraint(const cBulletPoinToPointConstraint &other) {
  }
  cBulletPoinToPointConstraint &operator=(const cBulletPoinToPointConstraint &other) {
    return *this;
  }

private:
  eConstraintType mConstraintType;
};

class cBulletHingeConstraint : public iConstraint {
public:
  cBulletHingeConstraint(iRigidBody *rb, const btVector3 &pivot, const btVector3 &axis)
      : iConstraint(CONSTRAINT_TYPE_HINGE) {
    mConstraint = new btHingeConstraint(*rb->GetBulletBody(), pivot, axis);
  }
  cBulletHingeConstraint(iRigidBody *rbA, iRigidBody *rbB, const btVector3 &pivotInA, const btVector3 &pivotInB,
                         const btVector3 &axisInA, const btVector3 &axisInB)
      : iConstraint(CONSTRAINT_TYPE_HINGE) {
    mConstraint =
        new btHingeConstraint(*rbA->GetBulletBody(), *rbB->GetBulletBody(), pivotInA, pivotInB, axisInA, axisInB);
  }
  virtual ~cBulletHingeConstraint() {
  }

  virtual btTypedConstraint *GetTypedConstraint() {
    return mConstraint;
  }

private:
  btHingeConstraint *mConstraint;
  cBulletHingeConstraint(const cBulletHingeConstraint &other) {
  }
  cBulletHingeConstraint &operator=(const iConstraint &other) {
    return *this;
  }

private:
  eConstraintType mConstraintType;
};
#pragma endregion
std::pair<std::string, std::string> lastColNames;

bool callbackFunc(btManifoldPoint &cp, const btCollisionObjectWrapper *obj1, int id1, int index1,
                  const btCollisionObjectWrapper *obj2, int id2, int index2) {
  ((iRigidBody *)obj1->getCollisionObject()->getUserPointer())->SetCollision(true);
  ((iRigidBody *)obj2->getCollisionObject()->getUserPointer())->SetCollision(true);
  lastColNames.first = ((iRigidBody *)obj1->getCollisionObject()->getUserPointer())->GetGOName();
  lastColNames.second = ((iRigidBody *)obj2->getCollisionObject()->getUserPointer())->GetGOName();
  return false;
}

struct PhysicsWorld {
  PhysicsWorld() {
    mCollisionConfiguration = new btDefaultCollisionConfiguration();
    mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
    mOverlappingPairCache = new btDbvtBroadphase();
    mSolver = new btSequentialImpulseConstraintSolver;
    mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration);
    mDynamicsWorld->setGravity(btVector3(0, -10, 0));
    gContactAddedCallback = callbackFunc;
  }
  ~PhysicsWorld() {
    if (mDynamicsWorld) {
      delete mDynamicsWorld;
      mDynamicsWorld = 0;
    }
    if (mSolver) {
      delete mSolver;
      mSolver = 0;
    }
    if (mOverlappingPairCache) {
      delete mOverlappingPairCache;
      mOverlappingPairCache = 0;
    }
    if (mDispatcher) {
      delete mDispatcher;
      mDispatcher = 0;
    }
    if (mCollisionConfiguration) {
      delete mCollisionConfiguration;
      mCollisionConfiguration = 0;
    }
  }
  void SetGravity(const vec3 &gravity) {
    mDynamicsWorld->setGravity(nConvert::ToBullet(gravity));
  }

  bool AddBody(iRigidBody *body) {
    iRigidBody *bulletRigidBody = dynamic_cast<iRigidBody *>(body);
    if (!bulletRigidBody) {
      return false;
    }
    mDynamicsWorld->addRigidBody(bulletRigidBody->GetBulletBody());
    return true;
  }
  bool RemoveBody(iRigidBody *body) {
    iRigidBody *bulletRigidBody = dynamic_cast<iRigidBody *>(body);
    if (!bulletRigidBody) {
      return false;
    }
    btRigidBody *BulletBtBody = bulletRigidBody->GetBulletBody();
    mDynamicsWorld->removeRigidBody(BulletBtBody);
    return true;
  }
  // Constraints
  virtual void AddConstraint(iConstraint *constraint) {
    eConstraintType type = constraint->GetConstraintType();

    switch (type) {
    case eConstraintType::CONSTRAINT_TYPE_POINT_TO_POINT: {
      cBulletPoinToPointConstraint *basConstraint = dynamic_cast<cBulletPoinToPointConstraint *>(constraint);
      this->mDynamicsWorld->addConstraint(basConstraint->GetTypedConstraint());

    } break;
    case eConstraintType::CONSTRAINT_TYPE_HINGE: {
      cBulletHingeConstraint *hingeConstraint = dynamic_cast<cBulletHingeConstraint *>(constraint);
      this->mDynamicsWorld->addConstraint(hingeConstraint->GetTypedConstraint());
    } break;
    default:
      break;
    }
  }
  virtual void RemoveConstraint(iConstraint *constraint) {
  }
  virtual std::pair<std::string, std::string> GetLastColPair() {
    return lastColNames;
  }
  virtual bool RayCast(vec3 &from, vec3 &to) {
    btCollisionWorld::ClosestRayResultCallback res(nConvert::ToBullet(from), nConvert::ToBullet(to));
    mDynamicsWorld->rayTest(nConvert::ToBullet(from), nConvert::ToBullet(to), res);
    return res.hasHit();
  }
  virtual iRigidBody *RayCastGetObject(vec3 &from, vec3 &to) {
    btCollisionWorld::ClosestRayResultCallback resObject(nConvert::ToBullet(from), nConvert::ToBullet(to));
    mDynamicsWorld->rayTest(nConvert::ToBullet(from), nConvert::ToBullet(to), resObject);
    // TODO: Return hit object
    // return dynamic_cast<nPhysics::cBulletRigidBody*>resObject.m_collisionObject->getUserPointer();
    if (resObject.hasHit()) {
      // return ((nPhysics::iRigidBody*)resObject.m_collisionObject->getUserPointer());
      return reinterpret_cast<iRigidBody *>(resObject.m_collisionObject->getUserPointer());
    } else {
      return NULL;
    }
    // return resObject.hasHit();
  }
  void Update(float dt) {
    // not working with 120hz monitor?
    mDynamicsWorld->stepSimulation(dt, 10);
  }

protected:
  PhysicsWorld(const PhysicsWorld &other) {
  }
  PhysicsWorld &operator=(const PhysicsWorld &other) {
    return *this;
  }

private:
  btDefaultCollisionConfiguration *mCollisionConfiguration;
  btCollisionDispatcher *mDispatcher;
  btBroadphaseInterface *mOverlappingPairCache;
  btSequentialImpulseConstraintSolver *mSolver = new btSequentialImpulseConstraintSolver;
  btDiscreteDynamicsWorld *mDynamicsWorld;
};

struct PhysicsFactory {
  virtual ~PhysicsFactory() {
  }
  virtual PhysicsWorld *CreatePhysicsWorld() {
    return new PhysicsWorld;
  }
  virtual iRigidBody *CreateRigidBody(const sRigidBodyDef &def, iShape *shape) {
    return new iRigidBody(def, shape);
  }
  // basicShapes
  virtual iSphereShape *CreateSphereShape(float radius) {
    return new iSphereShape(radius);
  }
  virtual iPlaneShape *CreatePlaneShape(const vec3 &normal, float constant) {
    return new iPlaneShape(normal, constant);
  }
  virtual iCylinderShape *CreateCylinderShape(const vec3 &halfExtent, int axis) {
    return new iCylinderShape(halfExtent, axis);
  }
  virtual iCapsuleShape *CreateCapsuleShape(float height, float radius, int axis) {
    return new iCapsuleShape(height, radius, axis);
  }
  virtual iBoxShape *CreateBoxShape(const vec3 &halfExtents) {
    return new iBoxShape(halfExtents);
  }
  // Contraints
  virtual iConstraint *CreatHingeConstraint(iRigidBody *rb, const vec3 &pivot, const vec3 &axis) {
    return new cBulletHingeConstraint(rb, nConvert::ToBullet(pivot), nConvert::ToBullet(axis));
  }
  virtual iConstraint *CreatHingeConstraint(iRigidBody *rbA, iRigidBody *rbB, const vec3 &pivotInA,
                                            const vec3 &pivotInB, const vec3 &axisInA, const vec3 &axisInB) {
    return new cBulletHingeConstraint(rbA, rbB, nConvert::ToBullet(pivotInA), nConvert::ToBullet(pivotInB),
                                      nConvert::ToBullet(axisInA), nConvert::ToBullet(axisInB));
  }
  virtual iConstraint *CreatePointToPointConstraint(iRigidBody *rb, const vec3 &pivot) {
    return new cBulletPoinToPointConstraint(rb, nConvert::ToBullet(pivot));
  }
  virtual iConstraint *CreatePointToPointConstraint(iRigidBody *rbA, iRigidBody *rbB, const vec3 &pivotInA,
                                                    const vec3 &pivotInB) {
    return new cBulletPoinToPointConstraint(rbA, rbB, nConvert::ToBullet(pivotInA), nConvert::ToBullet(pivotInB));
  }
};
#pragma endregion

HashMap<Guid, SharedPtr<iRigidBody>> data;

game::TransformComponent *cached_camera_xform;
SharedPtr<iRigidBody> sphere_rb;

void PhysicsBehavior::start() {
  assert(!world);
  assert(!factory);
  factory = new PhysicsFactory;
  world = factory->CreatePhysicsWorld();

  {
    // find camera
    auto &cameras = component::get_components_of_type<game::CameraComponent>();
    cached_camera_xform = component::get<game::TransformComponent>(cameras[0]->get_owner_id());
    iShape *new_shape = factory->CreateSphereShape(20.f);
    sRigidBodyDef def;
    def.Orientation = vec3(0);
    def.Position = cached_camera_xform->position;
    def.Mass = 40.f;
    sphere_rb = SharedPtr<iRigidBody>(factory->CreateRigidBody(def, new_shape));
    world->AddBody(sphere_rb.get());
  }

  // create physics objects
  auto &rigid_bodies = component::get_components_of_type<game::RigidBodyComponent>();
  for (auto &rb_ptr : rigid_bodies) {
    auto &rb = component::get_ref<game::RigidBodyComponent>(rb_ptr);
    auto xfrom_ptr = rb.transform.get();
    assert(xfrom_ptr);
    const auto &guid = rb.get_unique_id();
    sRigidBodyDef def;
    def.Orientation = xfrom_ptr->get_mesh_orientation_euler_angles();
    def.Position = xfrom_ptr->position;
    def.GameObjectName = utils::string::sprintf("object_%s", guid.to_string().c_str());

    def.Mass = 40.f; // rb.mass;
    iRigidBody *new_body = nullptr;
    iShape *new_shape = nullptr;
    switch (rb.type) {
    case game::RigidBodyType::Sphere:
      new_shape = factory->CreateSphereShape(rb.radius);
      break;
    case game::RigidBodyType::Box:
      new_shape = factory->CreateBoxShape(rb.half_extents);
      break;
    case game::RigidBodyType::Cylinder:
      new_shape = factory->CreateCylinderShape(rb.half_extents, rb.axis_id);
      break;
    case game::RigidBodyType::Plane:
      def.Mass = 0.0f;
      new_shape = factory->CreatePlaneShape(rb.plane_normal, rb.plane_constant);
      break;
    default:
      assert(false);
      break;
    }
    assert(new_shape);
    new_body = factory->CreateRigidBody(def, new_shape);
    world->AddBody(new_body);
    data[guid] = SharedPtr<iRigidBody>(new_body);
  }
  world->SetGravity(vec3(0, -9.8, 0));
}

void PhysicsBehavior::update(double delta_time) {
  world->Update(1 / 60.f);
  // update transforms
  auto &rigid_bodies = component::get_components_of_type<game::RigidBodyComponent>();

  sphere_rb->SetPosition(cached_camera_xform->position);
  for (auto &rb_ptr : rigid_bodies) {
    auto &rb = component::get_ref<game::RigidBodyComponent>(rb_ptr);
    const auto &guid = rb.get_unique_id();
    auto xfrom_ptr = rb.transform.get();
    assert(xfrom_ptr);
    if (data[guid]->GetShape()->GetShapeType() == nPhysics::SHAPE_TYPE_PLANE)
     continue;
    xfrom_ptr->position = data[guid]->GetPosition();

    xfrom_ptr->orient = glm::mat4(data[guid]->GetMatRotation());
  }
}

void PhysicsBehavior::destroy() {
  delete world;
  delete factory;
}
} // namespace behavior
