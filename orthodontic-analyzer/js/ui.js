export class UI {
  constructor(analyzer) {
    this.analyzer = analyzer;
    
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

    const upperArch = parseFloat(document.getElementById('upper-arch-length').value) || 0;
    const lowerArch = parseFloat(document.getElementById('lower-arch-length').value) || 0;

    return { upperWidths, lowerWidths, upperArchLength: upperArch, lowerArchLength: lowerArch };
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
  }

  loadDemoData() {
    // Some sensible defaults that would yield a slight crowding and normal bolton
    const upper = [0, 0, 10.5, 7.0, 7.5, 8.0, 7.0, 8.5, 8.5, 7.0, 8.0, 7.5, 7.0, 10.5, 0, 0];
    const lower = [0, 0, 11.0, 7.5, 7.5, 7.0, 6.0, 5.5, 5.5, 6.0, 7.0, 7.5, 7.5, 11.0, 0, 0];
    
    const upInputs = document.querySelectorAll('.upper-input');
    const lowInputs = document.querySelectorAll('.lower-input');

    upInputs.forEach((inp, i) => { if (upper[i]) inp.value = upper[i]; });
    lowInputs.forEach((inp, i) => { if (lower[i]) inp.value = lower[i]; });

    document.getElementById('upper-arch-length').value = 92.5; // Slightly less than needed
    document.getElementById('lower-arch-length').value = 85.0; // Slightly less than needed

    this.performAnalysis();
  }
}
