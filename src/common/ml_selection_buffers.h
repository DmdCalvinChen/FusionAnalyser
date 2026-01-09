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

#ifndef ML_SELECTION_BUFFERS
#define ML_SELECTION_BUFFERS

#include <QReadWriteLock>
#include <vector>

class MeshModel;
class MLSelectionBuffers
{
public:
	MLSelectionBuffers(MeshModel& m,unsigned int primitivebatch);
	~MLSelectionBuffers();

	enum ML_SELECTION_TYPE {ML_PERVERT_SEL = 0,ML_PERFACE_SEL = 1};

	void updateBuffer(ML_SELECTION_TYPE selbuf);
	void drawSelection(ML_SELECTION_TYPE selbuf) const;
	void deallocateBuffer(ML_SELECTION_TYPE selbuf);
	void setPointSize(float ptsz);
private:
	mutable QReadWriteLock _lock;

	MeshModel& _m;
	unsigned int _primitivebatch;
	typedef std::vector<GLuint> SelectionBufferNames;
	typedef std::vector< SelectionBufferNames > SelMap;
	SelMap _selmap;
	float _pointsize;
};

#endif
