using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using Newtonsoft.Json;
using System.IO;

public class SceneExport : MonoBehaviour
{
  public struct Quat
  {
    public float x, y, z, w;
    public static implicit operator Quat(UnityEngine.Quaternion inQuat)
    {
      var c = new Quat();
      c.x = inQuat.x;
      c.y = inQuat.y;
      c.z = inQuat.z;
      c.w = inQuat.w;
      return c;
    }
  }

  public struct Vector3
  {
    public float x, y, z;
    public static implicit operator Vector3(UnityEngine.Vector3 inVector)
    {
      var c = new Vector3();
      c.x = inVector.x;
      c.y = inVector.y;
      c.z = inVector.z;
      return c;
    }
  }
  struct Color
  {
    public float red, green, blue, alpha;
    public static implicit operator Color(UnityEngine.Color inColor)
    {
      var c = new Color();
      c.red = inColor.r;
      c.green = inColor.g;
      c.blue = inColor.b;
      c.alpha = inColor.a;
      return c;
    }
  }
  public struct Extents
  {
    public Vector3 min;
    public Vector3 max;
  }
  class Entity
  {
    // public Component[] components;
    public Dictionary<string, Component> components;
  }

  class Component
  {
  }
  class TransformComponent : Component
  {
    public Vector3 position;
    public Quat rotation;
    public Vector3 scale;
  }

  class StaticMeshComponent : Component
  {
    public string filename;
    public float importScaleFactor;
  }

  class RigidBodyComponent : Component
  {
    public enum Type
    {
      Box, Sphere, Capsule
    }
    public Extents extents;
    public float mass;
  }

  class MetadataComponent : Component
  {
    public string name;
  }

  class MaterialComponent : Component
  {
    public Color color;
    public float roughness;
    public float metal;
    public string diffuseFilename;
    public string normalFilename;
    public string occlusionRoughnessMetallic;
  }

  class CameraComponent : Component
  {
    public struct ClippingPlane
    {
      public float near;
      public float far;
    }
    public float fieldOfView;
    public ClippingPlane clippingPlane;
  }

  class DirectionalLightComponent : Component
  {
    public Color color;
    public float intensity;
  }

  class Scene
  {
    public string contentDirectory;
    public bool isLocalPath;
    public Entity[] entities;
  }

  [MenuItem("MyMenu/Export scene")]
  static void DoExport()
  {
    // collect game objects
    GameObject[] allObjects = UnityEngine.Object.FindObjectsOfType<GameObject>();
    var scene = new Scene();
    scene.contentDirectory = System.IO.Directory.GetParent(Application.dataPath).ToString().Replace('\\', '/');
    List<Entity> entities = new List<Entity>();
    List<string> paths = new List<string>();

    bool shouldCopyAssets = EditorUtility.DisplayDialog("Copy assets to the target folder?", "Do you want to export assets to the external location?", "Copy", "Keep them in Unity assets directory");

    foreach (var go in allObjects)
    {
      List<Component> components = new List<Component>();

      var cam = go.GetComponent<Camera>();
      if (cam)
      {
        var cameraComponent = new CameraComponent();
        cameraComponent.clippingPlane.near = cam.nearClipPlane;
        cameraComponent.clippingPlane.far = cam.farClipPlane;
        cameraComponent.fieldOfView = cam.fieldOfView;
        components.Add(cameraComponent);
      }

      var directionalLight = go.GetComponent<Light>();
      if (directionalLight)
      {
        if (directionalLight.type == LightType.Directional)
        {
          var directionalLightComponent = new DirectionalLightComponent();
          directionalLightComponent.intensity = directionalLight.intensity;
          directionalLightComponent.color = directionalLight.color;
          components.Add(directionalLightComponent);
        }
      }

      var meshRenderer = go.GetComponent<MeshRenderer>();
      if (meshRenderer)
      {
        // get material
        var material = meshRenderer.sharedMaterial;
        if (!material)
        {
          throw new System.Exception("Didn't find material");
        }
        if (material.shader.name != "Custom/DiscoEngine")
        {
          throw new System.Exception("Shader " + material.shader.name + " is not supported.");
        }
        var materialComponent = new MaterialComponent();
        materialComponent.color = material.GetColor("_Color");
        materialComponent.metal = material.GetFloat("_Metallness");
        materialComponent.roughness = material.GetFloat("_Roughness");

        materialComponent.diffuseFilename = AssetDatabase.GetAssetPath(material.GetTexture("_MainTex"));
        materialComponent.occlusionRoughnessMetallic = AssetDatabase.GetAssetPath(material.GetTexture("_OcclusionRoughnessMetallic"));
        materialComponent.normalFilename = AssetDatabase.GetAssetPath(material.GetTexture("_BumpMap"));

        paths.Add(materialComponent.diffuseFilename);
        paths.Add(materialComponent.occlusionRoughnessMetallic);
        paths.Add(materialComponent.normalFilename);

        if (shouldCopyAssets)
        {
          materialComponent.diffuseFilename = Path.GetFileName(materialComponent.diffuseFilename);
          materialComponent.occlusionRoughnessMetallic = Path.GetFileName(materialComponent.occlusionRoughnessMetallic);
          materialComponent.normalFilename = Path.GetFileName(materialComponent.normalFilename);
        }

        components.Add(materialComponent);

        var meshFilter = go.GetComponent<MeshFilter>();
        if (!meshFilter)
        {
          throw new System.Exception("Need mesh filter");
        }

        var staticMeshComponent = new StaticMeshComponent();
        staticMeshComponent.filename = AssetDatabase.GetAssetPath(meshFilter.sharedMesh);

        paths.Add(staticMeshComponent.filename);
        if (shouldCopyAssets)
        {
          staticMeshComponent.filename = Path.GetFileName(staticMeshComponent.filename);
        }

        var assetImporter = (ModelImporter)AssetImporter.GetAtPath(staticMeshComponent.filename);
        staticMeshComponent.importScaleFactor = 1.0f; // assetImporter.useFileScale ? 0.01f : 1.0f;
        components.Add(staticMeshComponent);
      }

      var collider = go.GetComponent<Collider>();
      if (collider)
      {
        var rigidBodyComponent = new RigidBodyComponent();
        rigidBodyComponent.extents.min = collider.bounds.min - go.transform.position;
        rigidBodyComponent.extents.max = collider.bounds.max - go.transform.position;
        rigidBodyComponent.mass = 0.0f;
        var rigidBody = go.GetComponent<Rigidbody>();
        if (rigidBody)
        {
          rigidBodyComponent.mass = rigidBody.mass;
        }
        components.Add(rigidBodyComponent);
      }

      if (components.Count == 0)
      {
        continue;
      }

      var transformComponent = new TransformComponent();
      transformComponent.position = go.transform.position;
      transformComponent.rotation = go.transform.rotation;
      transformComponent.scale = go.transform.lossyScale;
      components.Add(transformComponent);

      var metadataComponent = new MetadataComponent();
      metadataComponent.name = go.name;
      components.Add(metadataComponent);

      var entity = new Entity();
      entity.components = new Dictionary<string, Component>();
      foreach (var component in components)
      {
        entity.components[component.GetType().Name] = component;
      }
      entities.Add(entity);
    }


    scene.entities = entities.ToArray();
    scene.isLocalPath = false;
    string outPath = EditorUtility.SaveFilePanel("Save json", "", "scene.json", "json");

    if (shouldCopyAssets)
    {
      scene.isLocalPath = true;
      var sourcePath = scene.contentDirectory;
      scene.contentDirectory = "scene_assets/";

      // extract dir from path
      var workingDir = Path.GetDirectoryName(outPath);
      var contentDir = Path.Combine(workingDir, scene.contentDirectory);
      Directory.CreateDirectory(contentDir);

      // copy each asset which should be copied
      float progressBarIncrement = 1f / paths.Count;
      float progressBarTotal = 0f;

      foreach (var path in paths)
      {
        var sourceAssetPath = Path.Combine(sourcePath, path);
        string assetFilename = Path.GetFileName(sourceAssetPath);
        progressBarTotal += progressBarIncrement;
        EditorUtility.DisplayProgressBar("Copying assets", $"Copying: {assetFilename}", progressBarTotal);
        string targetPath = Path.Combine(contentDir, assetFilename);
        if (!File.Exists(targetPath))
        {
          File.Copy(sourceAssetPath, Path.Combine(contentDir, assetFilename));
        }
      }

      EditorUtility.ClearProgressBar();
    }
    string json = JsonConvert.SerializeObject(scene);

    System.IO.File.WriteAllText(outPath, json);
  }
}
