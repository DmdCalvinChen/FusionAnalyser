/****************************************************************************
* FusionAnalyser - Digital Dental Model Analysis Software
*
* Copyright (C) 2024-2026 AI-Align (基骨智能)
*
* This file is part of FusionAnalyser.
*
* FusionAnalyser is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* FusionAnalyser is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with FusionAnalyser. If not, see <https://www.gnu.org/licenses/>.
*
* SPDX-License-Identifier: GPL-3.0-or-later
****************************************************************************/

#ifndef ANALYSISREPORT_H
#define ANALYSISREPORT_H

#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <vector>

#include <treatmentpreviewguigroup/treatmentoverviewgui/tablearea.h>
#include <treatmentpreviewguigroup/treatmentoverviewgui/textarea.h>

#include "uicommon_global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class AnalysisReport;
}
QT_END_NAMESPACE

class AnalysisReport : public QWidget
{
    Q_OBJECT

public:
    AnalysisReport(QWidget* parent = nullptr);
    ~AnalysisReport();

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    /*构造绘制元素*/
    // 绘制区域的长宽，方便后续的pdf固定宽高导出
    void createDrawElements(int width_draw_area, int height_draw_area);

    // 导出pdf绘制
    void createDrawElementsForPdf(int width_draw_area, int height_draw_area);

    /*
     *   绘制背景
     */
    void drawBackground();

    /*
     * 放置全选、复制、导出、等顶层控件按钮
     */
    void placeTopButtons(int start_x, int& start_y, int width_draw_area, int height_draw_area);

    /*
     * 患者信息项填充,并修改下一个纵轴起始坐标
     */
    void fillPatientInfo(int start_x, int& start_y, int width_draw_area, int height_draw_area);

    /*
     * 牙位缺失信息填充
     */
    void fillToothFdiExistedInfo(int start_x, int& start_y, int width_draw_area,
                                 int height_draw_area);

    /*
     * 填充模型分析报告信息
     */
    void fillReportInfo(int start_x, int& start_y, int width_draw_area, int height_draw_area);

    /*
     * 填充模型分析报告信息(剩余信息 导出第二页pdf的高度初始化使用)
     */
    void fillReportRemainedInfo(int start_x, int& start_y, int width_draw_area,
                                int height_draw_area);

    /*
     * 填充图片
     */
    void fillImageInfo(int start_x, int& start_y, int width_draw_area, int height_draw_area);

    /*
     * 加载从工程数据获得的项设置状态bool值，进行对相关的selectbox状态初始化
     */
    void loadItemsStatusAndInitSelectBoxStatus();

    /*
     * 处理复选框与全选选中的状态信号关联逻辑
     */
    bool connect_once = false;    // 信号连接一次（首次）
    void connectSelectBoxsSignal();

    /*
     * 复选框选择性确认初始化check状态,根据项的描述匹配对应的复选框属性值
     */
    void initSelectBoxsCheckedStatus(std::vector<std::pair<QString, bool>> items_description);

    /*
     * 制作大标题
     */
    void makeMainTitle(int start_x, int& start_y, const QString& main_title, QFont* font = nullptr);

    /*
     * 制作选择box。可以替代大标题进行选择项导出
     */
    void makeMainTitleSelectBox(int start_x, int& start_y, const QString& prefix,
                                const QString& name_table);

    /*
     * 制作一个表格
     * 参数：显示坐标x，y； 表格属性名字；表格所有列顺序排列
     */
    void makeTableIncludingMainTitle(int start_x, int& start_y, int width_draw_area,
                                     const QString& _table_name,
                                     const std::vector<FormColumnData>& columns_data);

    // 组和制作序号 表格名字 表格所有列
    void make_item_func(
            int start_x, int& start_y, QString& order_prefix, QString& name_table,
            std::vector<FormColumnData>& columns_data,    // 第一个表格数据
            std::vector<FormColumnData>* columns_second_data =
                    nullptr    // 第二个表格数据（两个表格名字一样即第一个表格放不下时赋值）
    );

    // 返回大于0的字符串，否则返回 /， 单位可写
    template <typename T>
    QString getGoodData(T data, QString unit = "mm");
    template <>
    QString getGoodData(QString data, QString unit);

    /*
     *   清理绘制元素
     */
    void clearDrawElements();

    /*
     *   更新信息栏高度
     */
    void updateHeightItems();

public:
    /*
     *   设置以及标题颜色值
     *   @param color 标题颜色
     */
    void setColorMainTitle(const QColor& color);

    /*
     *   设置二级标题颜色值
     *   @param color 标题颜色
     */
    void setColorSubTitle(const QColor& color);

    /*
     *   设置一级主题字体
     *   @param font 字体
     */
    void setFontMainTitle(const QFont& font);

    /*
     *   设置二级主题字体
     *   @param font 字体
     */
    void setFontSubTitle(const QFont& font);

    /*
     *   设置单元格主题字体
     *   @param font 字体
     */
    void setFontTableSubTitle(const QFont& font);

    /*
     *   设置背景颜色
     *   @param color 颜色
     */
    void setColorBackground(const QColor& color);

    /*
     * 设置是不是本地版本的绘制模式
     */
    void setEnabeldLocalVersion(bool enabled = false)
    {
        is_local_version = enabled;
    }

signals:
    void updateDrawelements(std::vector<PrimitiveBase*>* draw_elements_);

public slots:
    /*
     *   更新患者界面数据槽
     */
    void updateGuiDataSlot();

    /*
     * pdf文件导出
     */
    void exportToPdfSlot();

    void updateElementsPreview();

    void selectBoxChangedSlot(QString text, bool check_state);

    void selectBoxEnabledSlot(QString text, bool enabled);

private:
    // 标题起始位置
    int start_x_main_title_ = 10;
    int blank_x_sub_title_ = 10;
    int start_y_title_ = 5;
    /*一级标题显示高度*/
    int height_H1_title_ = 40;
    /*二级标题显示高度*/
    int height_H2_title_ = 30;
    /*三级标题显示高度*/
    int height_H3_title_ = 25;
    /*纵向空白高度：控件间距*/
    int blank_y_ = 5;
    /*横向空白宽度：控件间距*/
    int blank_x_ = 50;
    // 文本区域
    int width_text_area_ = 200;
    // 绘制宽度
    int width_draw_area_;
    int height_text_area_ = 1200;

    // 时间轴每一项的高度
    int height_item_time_axis_ = 70;

    // 复选框起始空白宽度
    int blank_x_check_box_ = 5;

    // 一级标题字体、颜色
    QFont font_main_title_;
    QColor color_main_title_;
    // 二级标题字体、颜色
    QFont font_sub_title_;
    QFont font_table_sub_;
    QColor color_sub_title_;
    // 背景边框颜色
    QColor color_background_;

    /*绘制元素*/
    std::vector<PrimitiveBase*> draw_elements_;
    // 备注信息编辑框
    QTextEdit* note_text_edit_ = nullptr;
    // 导出pdf情况下渲染备注栏文字信息,其他情况影藏,包括其他元素
    bool export_pdf_status_ = false;
    // 复制，导出按钮
    QPushButton *copy_btn_ = nullptr, *export_btn_ = nullptr;

    bool is_local_version = false;

public:
    // 全选按钮
    QCheckBox* all_select_box_ = nullptr;
    // 选择项按钮
    std::map<QString, QCheckBox*> select_item_boxs_;
    // 设置选择框的宽度高度
    int width_check_box = 120, height_check_box = 25;
    // 项id累加值
    int id_item_add_ = 0;

    void setClearItemId()
    {
        id_item_add_ = 0;
    }
    QString getItemId()
    {
        return /*QString::number(++id_item_add_) + QString(".")*/ ("");
    }

private:
    Ui::AnalysisReport* ui;
};
#endif    // ANALYSISREPORT_H
