import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { STLLoader } from 'three/addons/loaders/STLLoader.js';

export class STLViewer {
  constructor(canvasId, containerId) {
    this.canvas = document.getElementById(canvasId);
    this.container = document.getElementById(containerId);
    this.upperMesh = null;
    this.lowerMesh = null;
    this.upperVisible = true;
    this.lowerVisible = true;

    this._initScene();
    this._initLights();
    this._initGrid();
    this._animate();
    this._handleResize();

    window.addEventListener('resize', () => this._handleResize());
    new ResizeObserver(() => this._handleResize()).observe(this.container);
  }

  _initScene() {
    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0x0a0e1a);
    this.scene.fog = new THREE.Fog(0x0a0e1a, 200, 400);

    const rect = this.container.getBoundingClientRect();
    const aspect = rect.width / rect.height || 1;

    this.camera = new THREE.PerspectiveCamera(45, aspect, 0.1, 1000);
    this.camera.position.set(0, 60, 100);

    this.renderer = new THREE.WebGLRenderer({
      canvas: this.canvas,
      antialias: true,
      alpha: true
    });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    this.renderer.setSize(rect.width, rect.height);
    this.renderer.shadowMap.enabled = true;
    this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
    this.renderer.toneMappingExposure = 1.2;

    this.controls = new OrbitControls(this.camera, this.canvas);
    this.controls.enableDamping = true;
    this.controls.dampingFactor = 0.08;
    this.controls.rotateSpeed = 0.8;
    this.controls.zoomSpeed = 1.2;
    this.controls.panSpeed = 0.8;
    this.controls.minDistance = 10;
    this.controls.maxDistance = 300;
    this.controls.target.set(0, 0, 0);
  }

  _initLights() {
    // Ambient for fill
    const ambient = new THREE.AmbientLight(0xffffff, 0.4);
    this.scene.add(ambient);

    // Hemisphere light for natural dental lighting
    const hemi = new THREE.HemisphereLight(0xcce0ff, 0x444422, 0.5);
    this.scene.add(hemi);

    // Key light from upper-front
    const keyLight = new THREE.DirectionalLight(0xfff5ee, 1.0);
    keyLight.position.set(30, 80, 60);
    keyLight.castShadow = true;
    keyLight.shadow.mapSize.width = 2048;
    keyLight.shadow.mapSize.height = 2048;
    this.scene.add(keyLight);

    // Fill light from left
    const fillLight = new THREE.DirectionalLight(0xe8f0ff, 0.5);
    fillLight.position.set(-50, 40, 30);
    this.scene.add(fillLight);

    // Rim light from behind
    const rimLight = new THREE.DirectionalLight(0xfff0e0, 0.3);
    rimLight.position.set(0, 20, -60);
    this.scene.add(rimLight);

    // Bottom fill for underside visibility
    const bottomLight = new THREE.DirectionalLight(0xe0e8ff, 0.2);
    bottomLight.position.set(0, -40, 20);
    this.scene.add(bottomLight);
  }

  _initGrid() {
    // Subtle ground plane
    const gridSize = 200;
    const gridDivisions = 40;
    const grid = new THREE.GridHelper(gridSize, gridDivisions, 0x1a2440, 0x111828);
    grid.position.y = -30;
    grid.material.opacity = 0.4;
    grid.material.transparent = true;
    this.scene.add(grid);

    // Ground shadow plane
    const planeGeo = new THREE.PlaneGeometry(gridSize, gridSize);
    const planeMat = new THREE.ShadowMaterial({ opacity: 0.15 });
    const plane = new THREE.Mesh(planeGeo, planeMat);
    plane.rotation.x = -Math.PI / 2;
    plane.position.y = -29.9;
    plane.receiveShadow = true;
    this.scene.add(plane);
  }

  _handleResize() {
    const rect = this.container.getBoundingClientRect();
    if (rect.width === 0 || rect.height === 0) return;
    this.camera.aspect = rect.width / rect.height;
    this.camera.updateProjectionMatrix();
    this.renderer.setSize(rect.width, rect.height);
  }

  _animate() {
    requestAnimationFrame(() => this._animate());
    this.controls.update();
    this.renderer.render(this.scene, this.camera);
  }

  _createMaterial(isUpper) {
    // Tooth-colored materials with subtle difference between upper and lower
    if (isUpper) {
      return new THREE.MeshPhysicalMaterial({
        color: 0xf5ede4,       // Warm off-white for upper
        metalness: 0.05,
        roughness: 0.35,
        clearcoat: 0.3,
        clearcoatRoughness: 0.4,
        reflectivity: 0.5,
        envMapIntensity: 0.6,
        side: THREE.DoubleSide
      });
    } else {
      return new THREE.MeshPhysicalMaterial({
        color: 0xe8ddd0,       // Slightly warmer/darker for lower
        metalness: 0.05,
        roughness: 0.38,
        clearcoat: 0.25,
        clearcoatRoughness: 0.45,
        reflectivity: 0.4,
        envMapIntensity: 0.5,
        side: THREE.DoubleSide
      });
    }
  }

  /**
   * Load STL from File object or URL string
   */
  async loadSTL(source, isUpper) {
    const loader = new STLLoader();

    return new Promise((resolve, reject) => {
      const onLoad = (geometry) => {
        // Remove old mesh
        const oldMesh = isUpper ? this.upperMesh : this.lowerMesh;
        if (oldMesh) {
          this.scene.remove(oldMesh);
          oldMesh.geometry.dispose();
          oldMesh.material.dispose();
        }

        // Compute normals for proper lighting
        geometry.computeVertexNormals();

        // Center the geometry
        geometry.computeBoundingBox();
        const center = new THREE.Vector3();
        geometry.boundingBox.getCenter(center);
        geometry.translate(-center.x, -center.y, -center.z);

        const material = this._createMaterial(isUpper);
        const mesh = new THREE.Mesh(geometry, material);
        mesh.castShadow = true;
        mesh.receiveShadow = true;

        if (isUpper) {
          this.upperMesh = mesh;
          mesh.visible = this.upperVisible;
        } else {
          this.lowerMesh = mesh;
          mesh.visible = this.lowerVisible;
        }

        this.scene.add(mesh);
        this._fitCamera();
        resolve();
      };

      if (typeof source === 'string') {
        // URL
        loader.load(source, onLoad, undefined, reject);
      } else {
        // File object
        const reader = new FileReader();
        reader.onload = (e) => {
          try {
            const geometry = loader.parse(e.target.result);
            onLoad(geometry);
          } catch (err) {
            reject(err);
          }
        };
        reader.onerror = reject;
        reader.readAsArrayBuffer(source);
      }
    });
  }

  _fitCamera() {
    const box = new THREE.Box3();
    let hasMesh = false;

    if (this.upperMesh && this.upperMesh.visible) {
      box.expandByObject(this.upperMesh);
      hasMesh = true;
    }
    if (this.lowerMesh && this.lowerMesh.visible) {
      box.expandByObject(this.lowerMesh);
      hasMesh = true;
    }

    // If nothing visible, try to use any available mesh
    if (!hasMesh) {
      if (this.upperMesh) { box.expandByObject(this.upperMesh); hasMesh = true; }
      if (this.lowerMesh) { box.expandByObject(this.lowerMesh); hasMesh = true; }
    }

    if (!hasMesh) return;

    const center = new THREE.Vector3();
    const size = new THREE.Vector3();
    box.getCenter(center);
    box.getSize(size);

    const maxDim = Math.max(size.x, size.y, size.z);
    const fov = this.camera.fov * (Math.PI / 180);
    let distance = maxDim / (2 * Math.tan(fov / 2));
    distance *= 1.5; // Some padding

    this.controls.target.copy(center);
    this.camera.position.set(
      center.x,
      center.y + distance * 0.4,
      center.z + distance
    );
    this.camera.lookAt(center);
    this.controls.update();
  }

  toggleUpper() {
    this.upperVisible = !this.upperVisible;
    if (this.upperMesh) this.upperMesh.visible = this.upperVisible;
    return this.upperVisible;
  }

  toggleLower() {
    this.lowerVisible = !this.lowerVisible;
    if (this.lowerMesh) this.lowerMesh.visible = this.lowerVisible;
    return this.lowerVisible;
  }

  resetView() {
    this._fitCamera();
  }
}
