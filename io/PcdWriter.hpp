/******************************************************************************
* Copyright (c) 2014, Brad Chambers (brad.chambers@gmail.com)
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

#pragma once

#include "PcdHeader.hpp"

#include <pdal/Writer.hpp>
#include <pdal/FlexWriter.hpp>
#include <pdal/Streamable.hpp>

namespace pdal
{

typedef std::shared_ptr<std::ostream> FileStreamPtr;

class PDAL_DLL PcdWriter : public FlexWriter, public Streamable
{
public:
    std::string getName() const;

    PcdWriter();
    ~PcdWriter()
    {}

protected:
    void finishOutput();

private:
    virtual void addArgs(ProgramArgs& args);
    virtual void initialize(PointTableRef table);
    virtual void readyFile(const std::string& filename,
        const SpatialReference& srs);
    virtual void writeView(const PointViewPtr view);
    virtual void doneFile();
    virtual bool processOne(PointRef& point);

    PcdHeader m_header;
    std::ostream *m_ostream;
    std::string m_filename;
    std::string m_compression_string;
    uint8_t m_compression;
    bool m_subtract_minimum;
    double m_offset_x;
    double m_offset_y;
    double m_offset_z;
    double m_scale_x;
    double m_scale_y;
    double m_scale_z;
    PointId m_idx;
    std::string m_curFilename;
    std::vector<char> m_pointBuf;

    bool fillPointBuf(PointRef& point, LeInserter& ostream);

    PcdWriter& operator=(const PcdWriter&); // not implemented
    PcdWriter(const PcdWriter&); // not implemented
};


} // namespaces
