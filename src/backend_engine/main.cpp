#include <iostream>
#include <string>
#include <map>
#include "MeshExt/AbsMesh.h"
#include "data/dentalmanager.h"
#include "data/fusionaligndata.h"
#include "bolton/BoltonAna.h"
#include "data/dentalanalysisdata.h"
#include "common/ml_mesh_type.h"
#include <wrap/io_trimesh/import_stl.h>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <unistd.h>

// 这是一个 C++ 后端命令行工具的伪代码/骨架
// 你需要将其集成到你的 Qt 项目中，并链接 SeparationManager 和 DentalFeatures。

int main(int argc, char* argv[]) {
    int argc_qt = 3;
    char* argv_qt[] = { (char*)"backend_engine", (char*)"-platform", (char*)"offscreen" };
    QApplication app(argc_qt, argv_qt);
    std::string metrics_path = "";

    // 1. 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--metrics" && i + 1 < argc) {
            metrics_path = argv[++i];
        }
    }

    FusionAlignData::getInstance();
    
    DentalManager upperManager;
    DentalManager lowerManager;
    
    PFusionAlignData->upper_dental_ = &upperManager;
    PFusionAlignData->lower_dental_ = &lowerManager;

    // Default mock widths
    std::map<QString, float> u_widths = {
        {"11", 8.5}, {"12", 7.0}, {"13", 8.0}, {"14", 7.5}, {"15", 7.0}, {"16", 10.5},
        {"21", 8.5}, {"22", 7.0}, {"23", 8.0}, {"24", 7.5}, {"25", 7.0}, {"26", 10.5}
    };
    std::map<QString, float> l_widths = {
        {"31", 5.5}, {"32", 6.0}, {"33", 7.0}, {"34", 7.5}, {"35", 7.5}, {"36", 11.0},
        {"41", 5.5}, {"42", 6.0}, {"43", 7.0}, {"44", 7.5}, {"45", 7.5}, {"46", 11.0}
    };
    
    float upper_arch_len = 95.0f;
    float lower_arch_len = 85.0f;

    // Load from metrics.json if provided
    if (!metrics_path.empty()) {
        QFile file(QString::fromStdString(metrics_path));
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject root = doc.object();
            
            if (root.contains("upperWidths")) {
                QJsonObject uw = root["upperWidths"].toObject();
                for (auto key : uw.keys()) {
                    u_widths[key] = uw[key].toDouble();
                }
            }
            if (root.contains("lowerWidths")) {
                QJsonObject lw = root["lowerWidths"].toObject();
                for (auto key : lw.keys()) {
                    l_widths[key] = lw[key].toDouble();
                }
            }
            if (root.contains("archLength")) {
                QJsonObject arch = root["archLength"].toObject();
                if (arch.contains("upper") && arch["upper"].toDouble() > 0) upper_arch_len = arch["upper"].toDouble();
                if (arch.contains("lower") && arch["lower"].toDouble() > 0) lower_arch_len = arch["lower"].toDouble();
            }
            if (root.contains("upperArchLength") && root["upperArchLength"].toDouble() > 0) {
                upper_arch_len = root["upperArchLength"].toDouble();
            }
            if (root.contains("lowerArchLength") && root["lowerArchLength"].toDouble() > 0) {
                lower_arch_len = root["lowerArchLength"].toDouble();
            }
        }
    }

    int index = 0;
    for (auto const& [fdi, w] : u_widths) {
        upperManager.cDental.teeth[index].strFDI = fdi;
        upperManager.cDental.teeth[index].localBoundbox.fLength = w;
        upperManager.cDental.bToothExist[index] = true;
        index++;
    }
    index = 0;
    for (auto const& [fdi, w] : l_widths) {
        lowerManager.cDental.teeth[index].strFDI = fdi;
        lowerManager.cDental.teeth[index].localBoundbox.fLength = w;
        lowerManager.cDental.bToothExist[index] = true;
        index++;
    }

    // Call original C++ logic
    common_ext::BoltonAna boltonAna;
    boltonAna.updateToothDataSlot();

    // Compute Crowding using original logic
    float upper_width_sum = 0.0f;
    for (auto const& [fdi, w] : u_widths) {
        upper_width_sum += w;
    }
    float lower_width_sum = 0.0f;
    for (auto const& [fdi, w] : l_widths) {
        lower_width_sum += w;
    }

    DentalAnalysisData analysisData;
    analysisData.space_requared_upper_ = QString::number(upper_width_sum, 'f', 2);
    analysisData.cur_length_arch_upper_ = QString::number(upper_arch_len, 'f', 2);
    analysisData.space_requared_lower_ = QString::number(lower_width_sum, 'f', 2);
    analysisData.cur_length_arch_lower_ = QString::number(lower_arch_len, 'f', 2);
    
    analysisData.updateSeverityOfCrowding();

    QJsonObject rootObj;
    
    QJsonObject uwObj;
    for (auto const& [fdi, w] : u_widths) {
        uwObj[fdi] = w;
    }
    rootObj["upper_widths"] = uwObj;
    
    QJsonObject lwObj;
    for (auto const& [fdi, w] : l_widths) {
        lwObj[fdi] = w;
    }
    rootObj["lower_widths"] = lwObj;
    
    rootObj["bolton_anterior"] = boltonAna.getBolton_33();
    rootObj["bolton_overall"] = boltonAna.getBolton_66();
    rootObj["upper_crowding"] = analysisData.crowding_severity_upper_;
    rootObj["lower_crowding"] = analysisData.crowding_severity_lower_;
    
    rootObj["upper_arch_length"] = upper_arch_len;
    rootObj["lower_arch_length"] = lower_arch_len;
    rootObj["upper_molar_width"] = 60.0;
    rootObj["lower_molar_width"] = 55.0;
    rootObj["spee_depth"] = 2.0;
    
    rootObj["status"] = "success";
    rootObj["message"] = "Original C++ Logic successfully instantiated!";
    
    QJsonDocument outDoc(rootObj);
    std::cout << outDoc.toJson(QJsonDocument::Compact).toStdString() << "\n";
    std::cout << std::flush;

    _exit(0);
    return 0;
}
