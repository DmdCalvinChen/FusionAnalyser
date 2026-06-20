import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { STLLoader } from 'three/addons/loaders/STLLoader.js';

export class Viewer {
  constructor(containerId) {
    this.container = document.getElementById(containerId);
    this.overlay = document.getElementById('loading-overlay');
    this.scene = new THREE.Scene();
    
    // Setup Camera
    this.camera = new THREE.PerspectiveCamera(45, this.container.clientWidth / this.container.clientHeight, 0.1, 1000);
    this.camera.position.set(0, 50, 100);

    // Setup Renderer
    this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    this.renderer.setSize(this.container.clientWidth, this.container.clientHeight);
    this.renderer.setPixelRatio(window.devicePixelRatio);
    this.container.appendChild(this.renderer.domElement);

    // Setup Controls
    this.controls = new OrbitControls(this.camera, this.renderer.domElement);
    this.controls.enableDamping = true;
    this.controls.dampingFactor = 0.1;

    // Lighting (Medical/Dental setup)
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
    this.scene.add(ambientLight);

    const dirLight1 = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight1.position.set(1, 1, 1);
    this.scene.add(dirLight1);

    const dirLight2 = new THREE.DirectionalLight(0xdcf0ff, 0.5);
    dirLight2.position.set(-1, -0.5, -1);
    this.scene.add(dirLight2);

    // Grid helper
    const gridHelper = new THREE.GridHelper(200, 50, 0x00d4aa, 0x222233);
    gridHelper.position.y = -30;
    gridHelper.material.opacity = 0.2;
    gridHelper.material.transparent = true;
    this.scene.add(gridHelper);

    // Materials
    this.upperMaterial = new THREE.MeshStandardMaterial({
      color: 0xf5f5f0, // slightly off-white tooth color
      roughness: 0.3,
      metalness: 0.1,
      side: THREE.DoubleSide
    });

    this.lowerMaterial = new THREE.MeshStandardMaterial({
      color: 0xeaeadd, // slightly different to distinguish
      roughness: 0.3,
      metalness: 0.1,
      side: THREE.DoubleSide
    });

    this.meshes = {
      upper: null,
      lower: null
    };

    // Resize handler
    window.addEventListener('resize', this.onWindowResize.bind(this), false);

    // Start render loop
    this.animate();
  }

  showLoading() {
    this.overlay.classList.remove('hidden');
  }

  hideLoading() {
    this.overlay.classList.add('hidden');
  }

  loadSTL(urlOrFile, type) {
    this.showLoading();
    
    if (this.meshes[type]) {
      this.scene.remove(this.meshes[type]);
      this.meshes[type].geometry.dispose();
      this.meshes[type] = null;
    }

    const loader = new STLLoader();
    
    const onLoad = (geometry) => {
      geometry.center(); // Center the geometry
      geometry.computeVertexNormals();

      const material = type === 'upper' ? this.upperMaterial : this.lowerMaterial;
      const mesh = new THREE.Mesh(geometry, material);
      
      // Default rotations if needed, STLs often come in weird orientations
      mesh.rotation.x = -Math.PI / 2;

      this.meshes[type] = mesh;
      this.scene.add(mesh);
      
      this.autoFitCamera();
      this.hideLoading();
    };

    if (typeof urlOrFile === 'string') {
      loader.load(urlOrFile, onLoad, undefined, (err) => {
        console.error(err);
        this.hideLoading();
        alert('加载模型失败');
      });
    } else {
      // It's a File object from input
      const reader = new FileReader();
      reader.onload = (e) => {
        const contents = e.target.result;
        try {
          const geometry = loader.parse(contents);
          onLoad(geometry);
        } catch(err) {
          console.error(err);
          this.hideLoading();
          alert('解析 STL 文件失败');
        }
      };
      reader.readAsArrayBuffer(urlOrFile);
    }
  }

  toggleMesh(type) {
    if (this.meshes[type]) {
      this.meshes[type].visible = !this.meshes[type].visible;
      return this.meshes[type].visible;
    }
    return false;
  }

  resetView() {
    this.controls.reset();
    this.autoFitCamera();
  }

  autoFitCamera() {
    // Basic auto-fit based on bounding boxes of loaded meshes
    const box = new THREE.Box3();
    let hasMesh = false;
    
    if (this.meshes.upper) { box.expandByObject(this.meshes.upper); hasMesh = true; }
    if (this.meshes.lower) { box.expandByObject(this.meshes.lower); hasMesh = true; }

    if (!hasMesh) return;

    const size = box.getSize(new THREE.Vector3());
    const center = box.getCenter(new THREE.Vector3());

    const maxDim = Math.max(size.x, size.y, size.z);
    const fov = this.camera.fov * (Math.PI / 180);
    let cameraZ = Math.abs(maxDim / 2 / Math.tan(fov / 2));
    
    cameraZ *= 1.5; // zoom out a bit

    this.camera.position.set(center.x, center.y + maxDim/2, center.z + cameraZ);
    this.camera.lookAt(center);
    this.controls.target.copy(center);
    this.controls.update();
  }

  onWindowResize() {
    this.camera.aspect = this.container.clientWidth / this.container.clientHeight;
    this.camera.updateProjectionMatrix();
    this.renderer.setSize(this.container.clientWidth, this.container.clientHeight);
  }

  animate() {
    requestAnimationFrame(this.animate.bind(this));
    this.controls.update();
    this.renderer.render(this.scene, this.camera);
  }
}
