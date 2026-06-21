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
      // geometry.center(); // Removed to preserve global coordinates and occlusal relationships
      geometry.computeVertexNormals();

      const material = type === 'upper' ? this.upperMaterial : this.lowerMaterial;
      const mesh = new THREE.Mesh(geometry, material);
      
      // Default rotations if needed, STLs often come in weird orientations
      mesh.rotation.x = -Math.PI / 2;

      this.meshes[type] = mesh;
      this.scene.add(mesh);
      
      this.autoFitCamera();
      this.hideLoading();

      // Dispatch event so UI knows models are loaded and can run calculations
      document.dispatchEvent(new CustomEvent('mesh-loaded'));
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

  computeMetrics() {
    const metrics = {};
    
    ['upper', 'lower'].forEach(type => {
      if (!this.meshes[type]) return;
      const pos = this.meshes[type].geometry.attributes.position;
      if (!pos || pos.count === 0) return;

      let minX = Infinity, maxX = -Infinity;
      let minY = Infinity, maxY = -Infinity;
      let minZ = Infinity, maxZ = -Infinity;

      for (let i = 0; i < pos.count; i += 10) { // step 10 for performance
        const x = pos.getX(i);
        const y = pos.getY(i);
        const z = pos.getZ(i);
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
        if (z < minZ) minZ = z;
        if (z > maxZ) maxZ = z;
      }

      // Arch Width (Molar Width) is roughly the X bounding box
      const molarWidth = Math.abs(maxX - minX) * 0.9; // 0.9 to account for crowns vs bone
      
      // Arch Length (Perimeter approximation)
      const archDepth = Math.abs(maxY - minY);
      // Rough perimeter of a parabola: W + 8/3 * (D^2 / W) -> simplified heuristic
      const archLength = molarWidth + (8/3) * (archDepth * archDepth / molarWidth);

      // Spee Curve Depth (only really meaningful for lower, but we can compute for both)
      // Algorithm: slice Y into 10 buckets, find max Z (cusp) in each bucket.
      const buckets = new Array(10).fill(-Infinity);
      const bucketSize = archDepth / 10;
      
      for (let i = 0; i < pos.count; i += 10) {
        const y = pos.getY(i);
        const z = pos.getZ(i);
        let bIdx = Math.floor((y - minY) / bucketSize);
        if (bIdx >= 10) bIdx = 9;
        if (bIdx < 0) bIdx = 0;
        if (z > buckets[bIdx]) buckets[bIdx] = z;
      }

      // The ends are incisors and molars. The middle is premolars.
      const zAnt = buckets[9]; // Assuming +Y is anterior (or vice versa, it's symmetric for Spee)
      const zPost = buckets[0]; 
      const zPlane = (zAnt + zPost) / 2;
      
      let maxDepth = 0;
      for (let i = 1; i < 9; i++) {
        // Depth is how far the middle cusps fall BELOW the plane.
        // Assuming maxZ is the occlusal surface.
        const expectedZ = zPost + (zAnt - zPost) * (i / 9);
        const depth = expectedZ - buckets[i];
        if (depth > maxDepth) maxDepth = depth;
      }

      // Safeguard: if depth is wild, clamp it to realistic bounds (0 to 5mm)
      let speeDepth = maxDepth;
      if (speeDepth < 0) speeDepth = 0.5; // practically flat
      if (speeDepth > 5) speeDepth = 5.0; // extreme deep curve

      metrics[type] = {
        molarWidth,
        archLength,
        speeDepth
      };
    });

    return metrics;
  }

  animate() {
    requestAnimationFrame(this.animate.bind(this));
    this.controls.update();
    this.renderer.render(this.scene, this.camera);
  }
}
