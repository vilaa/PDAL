/******************************************************************************
* Copyright (c) 2014, Brad Chambers (brad.chambers@gmail.com)
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

#include <pdal/PDALUtils.hpp>
#include <pdal/util/Algorithm.hpp>

#include <boost/algorithm/string/classification.hpp>

#include "PcdReader.hpp"
#include "PcdHeader.hpp"

namespace pdal
{

static StaticPluginInfo const s_info
{
    "readers.pcd",
    "Read data in the Point Cloud Library (PCL) format.",
    "http://pdal.io/stages/readers.pcd.html",
    { "pcd" }
};

CREATE_STATIC_STAGE(PcdReader, s_info)

std::string PcdReader::getName() const { return s_info.name; }

QuickInfo PcdReader::inspect()
{
    QuickInfo qi;

    initialize();

    for (auto i : m_header.m_fields)
        qi.m_dimNames.push_back(i.m_label);
    qi.m_pointCount = m_header.m_pointCount;
    qi.m_valid = true;

    return qi;
}

void PcdReader::ready(PointTableRef table)
{
    m_istream = Utils::openFile(m_filename, false);
    if (!m_istream)
        throwError("Unable to open text file '" + m_filename + "'.");

    m_istream->seekg(m_header.m_dataOffset);
}


void PcdReader::addDimensions(PointLayoutPtr layout)
{
    m_dims.clear();
    for (auto i : m_header.m_fields)
    {
        Dimension::BaseType base = Dimension::BaseType::None;
        if (i.m_type == PcdFieldType::U)
            base = Dimension::BaseType::Unsigned;
        else if (i.m_type == PcdFieldType::I)
            base = Dimension::BaseType::Signed;
        else if (i.m_type == PcdFieldType::F)
            base = Dimension::BaseType::Floating;
        Dimension::Type t = static_cast<Dimension::Type>(unsigned(base) | i.m_size);
        Utils::trim(i.m_label);
        Dimension::Id id = layout->registerOrAssignDim(i.m_label, t);
        if (Utils::contains(m_dims, id) && id != pdal::Dimension::Id::Unknown)
            throwError("Duplicate dimension '" + i.m_label +
                "' detected in input file '" + m_filename + "'.");
        m_dims.push_back(id);
    }
}


bool PcdReader::fillFields()
{
    while (true)
    {
        if (!m_istream->good())
            return false;

        std::string buf;

        std::getline(*m_istream, buf);
        //m_line++;
        if (buf.empty())
            continue;

        /*
        if (m_separator != ' ')
        {
            Utils::remove(buf, ' ');
            m_fields = Utils::split(buf, m_separator);
        }
        else
            m_fields = Utils::split2(buf, m_separator);
        */

        Utils::trim(buf);
        m_fields = Utils::split(buf, pdalboost::algorithm::is_any_of("\t\r "));
        if (m_fields.size() != m_dims.size())
        {
            log()->get(LogLevel::Error) << "Line " << //m_line <<
                " in '" << m_filename << "' contains " << m_fields.size() <<
                " fields when " << m_dims.size() << " were expected.  "
                "Ignoring." << std::endl;
            continue;
        }
        return true;
    }
}


bool PcdReader::processOne(PointRef& point)
{
    if (!fillFields())
        return false;

    double d;
    for (size_t i = 0; i < m_fields.size(); ++i)
    {
        if (!Utils::fromString(m_fields[i], d))
        {
            log()->get(LogLevel::Error) << "Can't convert "
                "field '" << m_fields[i] << "' to numeric value on line " <<
                /*m_line <<*/ " in '" << m_filename << "'.  Setting to 0." <<
                std::endl;
            d = 0;
        }
        point.setField(m_dims[i], d);
    }
    return true;
}


point_count_t PcdReader::read(PointViewPtr view, point_count_t count)
{
    PointId idx = view->size();
    point_count_t cnt = 0;
    PointRef point(*view, idx);
    while (cnt < count)
    {
        point.setPointId(idx);
        if (!processOne(point))
            break;
        cnt++;
        idx++;
    }
    return cnt;
}

void PcdReader::initialize()
{
    try {
        m_istream = Utils::openFile(m_filename, false);
        *m_istream >> m_header;
        //std::cout << m_header; // echo back for testing
    } catch (...) {
        Utils::closeFile(m_istream);
        throw;
    }

    Utils::closeFile(m_istream);
}


void PcdReader::done(PointTableRef table)
{
    Utils::closeFile(m_istream);
}


} // namespace pdal