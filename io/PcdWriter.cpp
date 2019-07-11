/******************************************************************************
* Copyright (c) 2011, Brad Chambers (brad.chambers@gmail.com)
* Copytight (c) 2016, Logan Byers (logan.c.byers@gmail.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include "PcdWriter.hpp"
#include "PcdHeader.hpp"

#include <pdal/PDALUtils.hpp>
#include <pdal/util/OStream.hpp>
#include <pdal/util/ProgramArgs.hpp>

namespace pdal
{

static StaticPluginInfo const s_info
{
    "writers.pcd",
    "Write data in the Point Cloud Library (PCL) format.",
    "http://pdal.io/stages/writers.pcd.html",
    { "pcd" }
};

CREATE_STATIC_STAGE(PcdWriter, s_info)

std::string PcdWriter::getName() const { return s_info.name; }

PcdWriter::PcdWriter() : m_ostream(NULL)
{}


void PcdWriter::addArgs(ProgramArgs& args)
{
    args.add("filename", "PCD output filename", m_filename).setPositional();
    args.add("compression", "Level of PCD compression to use "
        "(ascii, binary, compressed)", m_compression_string);
    args.add("subtract_minimum", "Set origin to minimum of XYZ dimension",
        m_subtract_minimum, true);
    args.add("offset_x", "Offset to be subtracted from XYZ position",
        m_offset_x, 0.0);
    args.add("offset_y", "Offset to be subtracted from XYZ position",
        m_offset_y, 0.0);
    args.add("offset_z", "Offset to be subtracted from XYZ position",
        m_offset_z, 0.0);
    args.add("scale_x", "Scale to divide from XYZ dimension", m_scale_x, 1.0);
    args.add("scale_y", "Scale to divide from XYZ dimension", m_scale_y, 1.0);
    args.add("scale_z", "Scale to divide from XYZ dimension", m_scale_z, 1.0);
}


void PcdWriter::initialize(PointTableRef table)
{
    m_header.m_version = PcdVersion::PCD_V7;
    m_header.m_height = 1;
    m_header.m_dataStorage = PcdDataStorage::ASCII;
    PcdField field;
    field.m_label = "X";
    field.m_id = Dimension::Id::X;
    field.m_size = 4;
    field.m_type = PcdFieldType::F;
    field.m_count = 1;
    m_header.m_fields.push_back(field);
 

    m_ostream = Utils::createFile(m_filename, true);
    if (!m_ostream)
        throwError("Couldn't open '" + m_filename + "' for output.");
}


void PcdWriter::readyFile(const std::string& filename,
    const SpatialReference& srs)
{
    m_idx = 0;
    std::cerr << "readyFile: " << filename << " " << m_filename << std::endl;
    std::ostream *out = Utils::createFile(filename, true);
    if (!out)
        throwError("Couldn't open '" + filename + "' for output.");
    m_curFilename = filename;
    Utils::writeProgress(m_progressFd, "READYFILE", filename);
}


bool PcdWriter::processOne(PointRef& point)
{
    LeInserter ostream(m_pointBuf.data(), m_pointBuf.size());
    if (!fillPointBuf(point, ostream))
        return false;
    m_ostream->write(m_pointBuf.data(), 0);
    return true;
}


bool PcdWriter::fillPointBuf(PointRef& point, LeInserter& ostream)
{
    ostream << point.getFieldAs<float>(Dimension::Id::X);
    ostream << point.getFieldAs<float>(Dimension::Id::Y);
    ostream << point.getFieldAs<float>(Dimension::Id::Z);
    return true;
}


void PcdWriter::writeView(const PointViewPtr view)
{
    Utils::writeProgress(m_progressFd, "READYVIEW",
        std::to_string(view->size()));

/*
    PointRef point(*view, 0);

    for (PointId idx = 0; idx < view->size(); ++idx)
    {
        point.setPointId(idx);
        processOne(point);
    }
*/
    Utils::writeProgress(m_progressFd, "DONEVIEW",
        std::to_string(view->size()));
}


void PcdWriter::doneFile()
{
    finishOutput();
    Utils::writeProgress(m_progressFd, "DONEFILE", m_curFilename);
    getMetadata().addList("filename", m_filename);
    delete m_ostream;
    m_ostream = NULL;
}


void PcdWriter::finishOutput()
{
    OLeStream out(m_ostream);

    //m_header.m_width = view->size();
    //m_header.m_pointCount = view->size();

    std::cerr << m_header << std::endl;
    
    out.seek(0);
    out << m_header;
    out.seek(m_header.m_dataOffset);
    
    m_ostream->flush();
}


} // namespaces
