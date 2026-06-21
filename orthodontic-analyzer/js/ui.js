export class UI {
  constructor(analyzer, viewer) {
    this.analyzer = analyzer;
    this.viewer = viewer;
    
    // Tooth arrays (FDI standard)
    this.upperTeeth = ['18','17','16','15','14','13','12','11', '21','22','23','24','25','26','27','28'];
    this.lowerTeeth = ['48','47','46','45','44','43','42','41', '31','32','33','34','35','36','37','38'];

    this.initGrids();
    this.bindEvents();
  }

  initGrids() {
    const upperRow = document.getElementById('upper-teeth-row');
    const lowerRow = document.getElementById('lower-teeth-row');

    const upperLeft = ['18','17','16','15','14','13','12','11'];
    const upperRight = ['21','22','23','24','25','26','27','28'];
    const lowerLeft = ['48','47','46','45','44','43','42','41'];
    const lowerRight = ['31','32','33','34','35','36','37','38'];

    const createRow = (leftIds, rightIds, jawName, cssClass) => {
      const tr = document.createElement('tr');
      
      leftIds.forEach(fdi => {
        tr.appendChild(this.createCell(fdi, cssClass));
      });

      const divider = document.createElement('td');
      divider.className = 'divider';
      tr.appendChild(divider);

      rightIds.forEach(fdi => {
        tr.appendChild(this.createCell(fdi, cssClass));
      });

      return tr;
    };

    upperRow.appendChild(createRow(upperLeft, upperRight, '上颌', 'upper'));
    lowerRow.appendChild(createRow(lowerLeft, lowerRight, '下颌', 'lower'));
  }

  createCell(fdi, jawClass) {
    const td = document.createElement('td');
    
    const input = document.createElement('input');
    input.type = 'number';
    input.step = '0.1';
    input.className = `tooth-input ${jawClass}-input`;
    input.dataset.fdi = fdi;
    input.placeholder = '-';
    
    td.appendChild(input);
    return td;
  }

  bindEvents() {
    document.getElementById('btn-calculate').addEventListener('click', () => {
      this.performAnalysis();
    });
  }

  collectData() {
    const upperWidths = {};
    const lowerWidths = {};

    document.querySelectorAll('.upper-input:not(:disabled)').forEach(input => {
      if (input.value) upperWidths[input.dataset.fdi] = parseFloat(input.value);
    });

    document.querySelectorAll('.lower-input:not(:disabled)').forEach(input => {
      if (input.value) lowerWidths[input.dataset.fdi] = parseFloat(input.value);
    });

    let upperArch = parseFloat(document.getElementById('upper-arch-length').value);
    let lowerArch = parseFloat(document.getElementById('lower-arch-length').value);
    let upperMolarWidth = parseFloat(document.getElementById('upper-molar-width').value);
    let lowerMolarWidth = parseFloat(document.getElementById('lower-molar-width').value);
    let speeDepth = parseFloat(document.getElementById('lower-spee-depth').value);

    return { 
      upperWidths, 
      lowerWidths, 
      upperArchLength: isNaN(upperArch) ? 0 : upperArch, 
      lowerArchLength: isNaN(lowerArch) ? 0 : lowerArch,
      upperMolarWidth: isNaN(upperMolarWidth) ? 0 : upperMolarWidth,
      lowerMolarWidth: isNaN(lowerMolarWidth) ? 0 : lowerMolarWidth,
      speeDepth: isNaN(speeDepth) ? 0 : speeDepth
    };
  }

  performAnalysis() {
    const data = this.collectData();
    const results = this.analyzer.calculate(data);
    
    this.renderResults(results);
  }

  renderResults(res) {
    // Helper to update a card
    const updateCard = (cardId, data, isPercent = true) => {
      const card = document.getElementById(cardId);
      const valSpan = card.querySelector('.value');
      const badge = card.querySelector('.status-badge');
      const msg = card.querySelector('.result-message');

      card.className = 'result-card'; // reset glow

      if (!data) {
        valSpan.innerText = '--';
        badge.innerText = '数据不足';
        badge.className = 'status-badge';
        msg.innerText = '';
        return;
      }

      valSpan.innerText = isPercent ? data.ratio.toFixed(2) : data.value.toFixed(1);
      
      let badgeText = '';
      if (data.status === 'normal') {
        badgeText = data.levelStr || '正常';
        card.classList.add('glow-green');
      } else if (data.status === 'warning') {
        badgeText = data.levelStr || '异常';
        card.classList.add('glow-yellow');
      } else {
        badgeText = data.levelStr || '异常';
        card.classList.add('glow-red');
      }

      badge.innerText = badgeText;
      badge.className = `status-badge ${data.status}`;
      
      msg.innerText = data.message;
      msg.className = `result-message text-${data.status}`;
    };

    updateCard('card-bolton-anterior', res.boltonAnterior, true);
    updateCard('card-bolton-overall', res.boltonOverall, true);
    updateCard('card-crowding-upper', res.upperCrowding, false);
    updateCard('card-crowding-lower', res.lowerCrowding, false);
    
    // Arch Dimensions is special
    const archCard = document.getElementById('card-arch-dimensions');
    if (res.archDimensions) {
      updateCard('card-arch-dimensions', res.archDimensions, false);
      archCard.querySelector('.value').innerText = '展示';
      archCard.querySelector('.result-detail').innerText = `上颌: ${res.archDimensions.upperMsg} | 下颌: ${res.archDimensions.lowerMsg}`;
      
      this.drawArchCurve(
        res.archDimensions.upperMolarWidth, res.archDimensions.upperArchLength,
        res.archDimensions.lowerMolarWidth, res.archDimensions.lowerArchLength
      );
    } else {
      updateCard('card-arch-dimensions', null, false);
      archCard.querySelector('.result-detail').innerText = '上颌: -- | 下颌: --';
      const canvas = document.getElementById('arch-curve-canvas');
      if (canvas) canvas.style.display = 'none';
    }

    updateCard('card-spee-curve', res.speeCurve, false);
  }

  async loadDemoData() {
    // We now rely on the Python backend API to perform real 3D segmentation (via C++)
    // Clear all inputs first
    const inputs = document.querySelectorAll('input[type="number"]');
    inputs.forEach(inp => inp.value = '');
    
    // We do not call performAnalysis here directly because we need the viewer to load the STLs
    // The main.js will call viewer.loadSTL, and when 'mesh-loaded' fires, we will trigger the API.
  }

  async fetchBackendData(upperFile, lowerFile) {
    try {
      const btn = document.getElementById('btn-calculate');
      const origText = btn.innerText;
      btn.innerText = '计算中...';
      btn.disabled = true;

      // Ensure viewer has meshes loaded to get base64
      const response = await fetch('/api/analyze', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          // In a real app, we would extract the STL binary or base64 from the viewer or file input.
          // For now, since the Python server runs locally, we can just pass a flag to tell it to process the local example STLs
          // if we don't pass base64. Let's pass the base64 if we can, or just tell the server to load the local files.
          upper_stl_b64: window.upperStlBase64 || "",
          lower_stl_b64: window.lowerStlBase64 || "",
          metrics: this.collectData()
        })
      });

      const res = await response.json();
      
      if (res.status === 'success') {
        const data = res.data;
        // Populate inputs with real data from backend
        if (data.upper_widths) {
          for (const [fdi, width] of Object.entries(data.upper_widths)) {
            const inp = document.querySelector(`.upper-input[data-fdi="${fdi}"]`);
            if (inp) inp.value = width.toFixed(1);
          }
        }
        if (data.lower_widths) {
          for (const [fdi, width] of Object.entries(data.lower_widths)) {
            const inp = document.querySelector(`.lower-input[data-fdi="${fdi}"]`);
            if (inp) inp.value = width.toFixed(1);
          }
        }

        // Arch dimensions and Spee can be populated here if backend returns them
        if (data.upper_arch_length !== undefined) {
          const inp = document.getElementById('upper-arch-length');
          if (inp) inp.value = data.upper_arch_length.toFixed(1);
        }
        if (data.lower_arch_length !== undefined) {
          const inp = document.getElementById('lower-arch-length');
          if (inp) inp.value = data.lower_arch_length.toFixed(1);
        }
        if (data.upper_molar_width !== undefined) {
          const inp = document.getElementById('upper-molar-width');
          if (inp) inp.value = data.upper_molar_width.toFixed(1);
        }
        if (data.lower_molar_width !== undefined) {
          const inp = document.getElementById('lower-molar-width');
          if (inp) inp.value = data.lower_molar_width.toFixed(1);
        }
        if (data.spee_depth !== undefined) {
          const inp = document.getElementById('lower-spee-depth');
          if (inp) inp.value = data.spee_depth.toFixed(1);
        }
        
        this.backendResults = {
            boltonAnterior: {
                ratio: parseFloat(data.bolton_anterior) || 0,
                status: 'normal',
                levelStr: data.bolton_anterior,
                message: '基于 C++ 后端 BoltonAna 类'
            },
            boltonOverall: {
                ratio: parseFloat(data.bolton_overall) || 0,
                status: 'normal',
                levelStr: data.bolton_overall,
                message: '基于 C++ 后端 BoltonAna 类'
            },
            upperCrowding: {
                value: parseFloat(data.upper_crowding) || 0,
                status: data.upper_crowding.includes('III') ? 'abnormal' : (data.upper_crowding.includes('II') ? 'warning' : (data.upper_crowding.includes('I') ? 'warning' : 'normal')),
                levelStr: data.upper_crowding,
                message: '基于 C++ 后端 DentalAnalysisData 类'
            },
            lowerCrowding: {
                value: parseFloat(data.lower_crowding) || 0,
                status: data.lower_crowding.includes('III') ? 'abnormal' : (data.lower_crowding.includes('II') ? 'warning' : (data.lower_crowding.includes('I') ? 'warning' : 'normal')),
                levelStr: data.lower_crowding,
                message: '基于 C++ 后端 DentalAnalysisData 类'
            }
        };
      } else {
        alert("后端计算错误: " + res.message);
      }
    } catch (e) {
      console.error(e);
      alert("连接后端服务失败，请确保 server.py 正在运行！");
    } finally {
      const btn = document.getElementById('btn-calculate');
      btn.innerText = '进行分析计算';
      btn.disabled = false;
      
      // Perform the local frontend analysis using the populated data
      this.performAnalysisLocal();
    }
  }

  performAnalysisLocal() {
    const data = this.collectData();
    const results = this.analyzer.calculate(data);
    
    // Override the js calculated results with the backend true results
    if (this.backendResults) {
        results.boltonAnterior = this.backendResults.boltonAnterior;
        results.boltonOverall = this.backendResults.boltonOverall;
        results.upperCrowding = this.backendResults.upperCrowding;
        results.lowerCrowding = this.backendResults.lowerCrowding;
    }

    this.renderResults(results);
  }

  performAnalysis() {
    // When called externally, attempt to fetch backend data first
    this.fetchBackendData();
  }

  drawArchCurve(upperW, upperP, lowerW, lowerP) {
    const canvas = document.getElementById('arch-curve-canvas');
    if (!canvas) return;
    canvas.style.display = 'inline-block';
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Setup coordinates: center top is origin (incisors)
    const scale = 2.0; // 1mm = 2px
    const centerX = canvas.width / 2;
    const topY = 20;

    const drawParabola = (W, P, color, label) => {
      // Basic safeguard: Arch perimeter P must be slightly greater than width W
      if (W <= 0 || P <= W) return;
      
      // Mathematical approximation of Parabola arc length to find depth D
      // P ~ W + (8/3)*(D^2 / W)  => D = sqrt( (P - W) * W * 0.375 )
      const D = Math.sqrt((P - W) * W * 0.375);
      
      ctx.beginPath();
      ctx.strokeStyle = color;
      ctx.lineWidth = 3;
      
      const a = D / Math.pow(W/2, 2);
      
      for (let x = -W/2; x <= W/2; x += 0.5) {
        const y = a * x * x;
        const px = centerX + x * scale;
        const py = topY + y * scale;
        if (x === -W/2) ctx.moveTo(px, py);
        else ctx.lineTo(px, py);
      }
      ctx.stroke();

      // Labeling
      ctx.fillStyle = color;
      ctx.font = '12px "Inter", sans-serif';
      ctx.fillText(label, centerX + (W/2)*scale - 30, topY + D*scale + 15);
    };

    drawParabola(upperW, upperP, '#00d4aa', '上颌 (U)');
    drawParabola(lowerW, lowerP, '#4a90e2', '下颌 (L)');
  }
}
