// Orthodontic Analysis Engine

export class Analyzer {
  constructor() {
    // Bolton standard values
    this.BOLTON_ANTERIOR_STD = 78.8; // 78.8%
    this.BOLTON_ANTERIOR_MIN = 77.08;
    this.BOLTON_ANTERIOR_MAX = 80.52;
    
    this.BOLTON_OVERALL_STD = 91.5; // 91.5%
    this.BOLTON_OVERALL_MIN = 89.99;
    this.BOLTON_OVERALL_MAX = 93.01;
  }

  /**
   * data: {
   *   upperWidths: { '18': val, '17': val, ... },
   *   lowerWidths: { '48': val, '47': val, ... },
   *   upperArchLength: val,
   *   lowerArchLength: val
   * }
   */
  calculate(data) {
    const results = {};

    // 1. Bolton Anterior (3-3)
    const upper33 = this.sumWidths(data.upperWidths, ['13', '12', '11', '21', '22', '23']);
    const lower33 = this.sumWidths(data.lowerWidths, ['43', '42', '41', '31', '32', '33']);
    
    if (upper33 > 0 && lower33 > 0) {
      const ratio = (lower33 / upper33) * 100;
      let status = 'normal';
      let message = '前牙比例协调';
      
      if (ratio < this.BOLTON_ANTERIOR_MIN) {
        status = 'abnormal';
        const excess = upper33 - (lower33 / (this.BOLTON_ANTERIOR_STD / 100));
        message = `上颌前牙牙量相对过大，多余约 ${excess.toFixed(1)} mm`;
      } else if (ratio > this.BOLTON_ANTERIOR_MAX) {
        status = 'abnormal';
        const excess = lower33 - (upper33 * (this.BOLTON_ANTERIOR_STD / 100));
        message = `下颌前牙牙量相对过大，多余约 ${excess.toFixed(1)} mm`;
      }

      results.boltonAnterior = { ratio, status, message };
    } else {
      results.boltonAnterior = null; // Missing data
    }

    // 2. Bolton Overall (6-6)
    const upper66 = this.sumWidths(data.upperWidths, ['16', '15', '14', '13', '12', '11', '21', '22', '23', '24', '25', '26']);
    const lower66 = this.sumWidths(data.lowerWidths, ['46', '45', '44', '43', '42', '41', '31', '32', '33', '34', '35', '36']);

    if (upper66 > 0 && lower66 > 0) {
      const ratio = (lower66 / upper66) * 100;
      let status = 'normal';
      let message = '全牙比例协调';
      
      if (ratio < this.BOLTON_OVERALL_MIN) {
        status = 'abnormal';
        const excess = upper66 - (lower66 / (this.BOLTON_OVERALL_STD / 100));
        message = `上颌全牙牙量相对过大，多余约 ${excess.toFixed(1)} mm`;
      } else if (ratio > this.BOLTON_OVERALL_MAX) {
        status = 'abnormal';
        const excess = lower66 - (upper66 * (this.BOLTON_OVERALL_STD / 100));
        message = `下颌全牙牙量相对过大，多余约 ${excess.toFixed(1)} mm`;
      }

      results.boltonOverall = { ratio, status, message };
    } else {
      results.boltonOverall = null;
    }

    // 3. Upper Crowding (7-7 usually, but we sum whatever is available)
    const upperTotalWidth = this.sumAllWidths(data.upperWidths);
    if (upperTotalWidth > 0 && data.upperArchLength > 0) {
      const crowding = upperTotalWidth - data.upperArchLength;
      results.upperCrowding = this.classifyCrowding(crowding);
    } else {
      results.upperCrowding = null;
    }

    // 4. Lower Crowding
    const lowerTotalWidth = this.sumAllWidths(data.lowerWidths);
    if (lowerTotalWidth > 0 && data.lowerArchLength > 0) {
      const crowding = lowerTotalWidth - data.lowerArchLength;
      results.lowerCrowding = this.classifyCrowding(crowding);
    } else {
      results.lowerCrowding = null;
    }

    return results;
  }

  sumWidths(widthsDict, fdiList) {
    let sum = 0;
    for (const fdi of fdiList) {
      const val = widthsDict[fdi];
      if (typeof val === 'number' && !isNaN(val)) {
        sum += val;
      }
    }
    return sum;
  }

  sumAllWidths(widthsDict) {
    let sum = 0;
    for (const fdi in widthsDict) {
      const val = widthsDict[fdi];
      if (typeof val === 'number' && !isNaN(val)) {
        sum += val;
      }
    }
    return sum;
  }

  classifyCrowding(crowdingValue) {
    let status = 'normal';
    let levelStr = '正常';
    let message = '牙列间隙正常';

    if (crowdingValue < -0.5) {
      status = 'normal'; // It's spacing, showing blue normally, we map to normal style or custom
      levelStr = '散在间隙';
      message = '存在多余间隙';
    } else if (crowdingValue >= -0.5 && crowdingValue <= 0.5) {
      status = 'normal';
      levelStr = '正常';
      message = '牙量与骨量基本协调';
    } else if (crowdingValue > 0.5 && crowdingValue <= 4) {
      status = 'warning';
      levelStr = 'I 度拥挤 (轻度)';
      message = '轻度拥挤，可能需要片切或轻度扩弓';
    } else if (crowdingValue > 4 && crowdingValue <= 8) {
      status = 'warning'; // map to orange visually in css
      levelStr = 'II 度拥挤 (中度)';
      message = '中度拥挤，可能需要拔牙或远中移动';
    } else if (crowdingValue > 8) {
      status = 'abnormal';
      levelStr = 'III 度拥挤 (重度)';
      message = '重度拥挤，通常需要拔牙矫治';
    }

    return { value: crowdingValue, status, levelStr, message };
  }
}
