import { Viewer } from './viewer.js?v=10';
import { Analyzer } from './analyzer.js?v=10';
import { UI } from './ui.js?v=10';

document.addEventListener('DOMContentLoaded', () => {
  const viewer = new Viewer('viewer-container');
  const analyzer = new Analyzer();
  const ui = new UI(analyzer, viewer);

  // File Upload Handlers
  document.getElementById('upload-upper').addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
      viewer.loadSTL(e.target.files[0], 'upper');
    }
  });

  document.getElementById('upload-lower').addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
      viewer.loadSTL(e.target.files[0], 'lower');
    }
  });

  // Toggle buttons
  document.getElementById('btn-toggle-upper').addEventListener('click', (e) => {
    const isVisible = viewer.toggleMesh('upper');
    e.target.classList.toggle('active', isVisible);
  });

  document.getElementById('btn-toggle-lower').addEventListener('click', (e) => {
    const isVisible = viewer.toggleMesh('lower');
    e.target.classList.toggle('active', isVisible);
  });

  // Reset View
  document.getElementById('btn-reset-view').addEventListener('click', () => {
    viewer.resetView();
  });

  // Handle Demo Data
  document.getElementById('btn-load-demo').addEventListener('click', () => {
    // Fill mock tooth widths only, clear others
    ui.loadDemoData(); 
    viewer.loadSTL('example/upper.stl', 'upper');
    viewer.loadSTL('example/lower.stl', 'lower');
  });

  // Re-calculate analysis whenever a mesh finishes loading
  document.addEventListener('mesh-loaded', () => {
    ui.performAnalysis();
  });

  // Initial prompt
  console.log("Orthodontic Analyzer initialized.");
});
