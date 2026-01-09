/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include "ui_savemaskexporter.h"
#include "savemaskexporter.h"
#include "changetexturename.h"

#include <QFileInfo>

SaveMaskExporterDialog::SaveMaskExporterDialog(QWidget *parent, MeshModel *m, int capability, int defaultBits, RichParameterSet *_parSet, GLArea* glar) :
	QDialog(parent), m(m), mask(0), capability(capability), defaultBits(defaultBits), parSet(_parSet), glar(glar)
{
	ui = new Ui::MaskExporterDialog();
	InitDialog();
}

void SaveMaskExporterDialog::InitDialog()
{
	SaveMaskExporterDialog::ui->setupUi(this);
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(SlotOkButton()));
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(SlotCancelButton()));
	stdParFrame = new StdParFrame(this, glar);
	stdParFrame->loadFrameContent(*parSet);
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->addWidget(stdParFrame);
	ui->saveParBox->setLayout(vbox);
	QFileInfo fi(m->fullName());
	this->setWindowTitle(tr("export mesh"));
	// Show the additional parameters only for formats that have some.
	if (parSet->isEmpty()) ui->saveParBox->hide();
	else ui->saveParBox->show();
	//all - none
	//ui->AllButton->setChecked(true);
	//ui->NoneButton->setChecked(true);

	SetTextureName();
	SetMaskCapability();
}

void SaveMaskExporterDialog::SetTextureName()
{
	if (m->cm.textures.size() == 0)
	{
	}

	for (unsigned int i = 0; i < m->cm.textures.size(); i++)
	{
		QString item(m->cm.textures[i].c_str());
		//ui->listTextureName->addItem(item);
	}
}

int SaveMaskExporterDialog::GetNewMask()
{
	return this->mask;
}

bool SaveMaskExporterDialog::shouldBeChecked(int bit, int /*capabilityBits*/, int defaultBits)
{
	if (!m->hasDataMask(MeshModel::io2mm(bit))) return false;
	//if( (bit & meshBits) == 0 ) return false;
	if ((bit & defaultBits) == 0) return false;
	return true;
}

bool SaveMaskExporterDialog::shouldBeEnabled(int iobit, int capabilityBits, int /*defaultBits*/)
{
	if ((iobit & capabilityBits) == 0) return false;
	int mmbit = MeshModel::io2mm(iobit);
	if (!m->hasDataMask(mmbit)) return false;
	return true;
}

void SaveMaskExporterDialog::checkAndEnable(QCheckBox *qcb, int bit, int capabilityBits, int defaultBits)
{
	qcb->setEnabled(shouldBeEnabled(bit, capabilityBits, defaultBits));
	qcb->setChecked(shouldBeChecked(bit, capabilityBits, defaultBits));
}

void SaveMaskExporterDialog::SetMaskCapability()
{
	//vert
	// point cloud fix: if a point cloud, probably you'd want to save vertex normals
// 	if ((m->cm.fn == 0) && (m->cm.en == 0))
// 		ui->check_iom_vertnormal->setChecked(true);

	//face
	//wedge
	//checkAndEnable(ui->check_iom_polygonal, vcg::tri::io::Mask::IOM_BITPOLYGONAL, capability, defaultBits);

	//camera THIS ONE HAS TO BE CORRECTED !!!!
// 	if (capability == 0)
// 		ui->NoneButton->setChecked(true);
}

void SaveMaskExporterDialog::updateMask()
{
	int newmask = 0;
//
// 	if (ui->check_iom_camera->isChecked()) { newmask |= vcg::tri::io::Mask::IOM_CAMERA; }
// 	if (ui->check_iom_polygonal->isChecked()) { newmask |= vcg::tri::io::Mask::IOM_BITPOLYGONAL; }

// 	for (unsigned int i = 0; i < m->cm.textures.size(); i++)
// 		m->cm.textures[i] = ui->listTextureName->item(i)->text().toStdString();
	this->mask = newmask;
}

//slot
void SaveMaskExporterDialog::SlotOkButton()
{
	updateMask();
	stdParFrame->readValues(*parSet);
}

void SaveMaskExporterDialog::SlotCancelButton()
{
	this->mask = -1;
}

void SaveMaskExporterDialog::SlotRenameTexture()
{
}

void SaveMaskExporterDialog::SlotSelectionTextureName()
{
}

void SaveMaskExporterDialog::SlotSelectionAllButton()
{
	//vert
	//face
	//wedg
	//camera
	//ui->check_iom_camera->setChecked(ui->check_iom_camera->isEnabled());
}

void SaveMaskExporterDialog::SlotSelectionNoneButton()
{
	//vert
//
// 	//face
	//wedg
	//camera
	//ui->check_iom_camera->setChecked(false);
}

void SaveMaskExporterDialog::on_check_help_stateChanged(int)
{
	stdParFrame->toggleHelp();
}

SaveMaskExporterDialog::~SaveMaskExporterDialog()
{
	delete ui;
}
