#ifndef __CBUFFER_INCLUDED__
#define __CBUFFER_INCLUDED__

#define MAKE_MOTION_VECTOR_PASS 1

cbuffer CameraParameters : register(b0) {
  float4x4 projectionMatrix;
  float4x4 viewMatrix;

  float4 _timeConstant;
  
#if MAKE_MOTION_VECTOR_PASS
  float4x4 viewMatrixPrevious;
#endif
};

cbuffer ModelParameters : register(b1) {
  float4x4 modelMatrix;
  float4x4 modelMatrixPrev;
};


#endif //
