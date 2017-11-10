/******************************************************************************
 * Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
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

#include <pdal/pdal_internal.hpp>

#include "CmapFilter.hpp"
#include <pdal/PointView.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/pdal_macros.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

namespace pdal
{

static PluginInfo const s_info =
    PluginInfo("filters.cmap", "Colormap stuff",
               "http://pdal.io/stages/filters.cmap.html");

CREATE_SHARED_PLUGIN(1, 0, CmapFilter, Filter, s_info)

const char* m_source = R"(
import matplotlib
import matplotlib.pyplot as plt
import numpy as np


def filter(ins, outs):
    ret = ins[pdalargs['dimension']]
    cmap = plt.get_cmap(pdalargs['cmap'])
    if pdalargs.get('norm') != None:
        if pdalargs['norm'] == 'log':
            norm = matplotlib.colors.LogNorm()
    else:
        norm = matplotlib.colors.Normalize()
    norm.autoscale(ret)

    rgba = cmap(norm(ret))
    Red = np.round(rgba[:, 0] * 256)
    Green = np.round(rgba[:, 1] * 256)
    Blue = np.round(rgba[:, 2] * 256)

    outs['Red'] = Red.astype('uint16')
    outs['Green'] = Green.astype('uint16')
    outs['Blue'] = Blue.astype('uint16')
    return True
)";

const char* m_pdalargs =
    "{\"cmap\":\"PuOr\",\"dimension\":\"HeightAboveGround\"}";

std::string CmapFilter::getName() const
{
    return s_info.name;
}

void CmapFilter::addArgs(ProgramArgs& args)
{
}

void CmapFilter::addDimensions(PointLayoutPtr layout)
{
    layout->registerDims(
        {Dimension::Id::Red, Dimension::Id::Green, Dimension::Id::Blue});
}

void CmapFilter::ready(PointTableRef table)
{
    static_cast<plang::Environment*>(plang::Environment::get())
        ->set_stdout(log()->getLogStream());
    m_script = new plang::Script(m_source, "anything", "filter");
    m_pythonMethod = new plang::Invocation(*m_script);
    m_pythonMethod->compile();
    m_totalMetadata = table.metadata();
}

PointViewSet CmapFilter::run(PointViewPtr view)
{
    log()->get(LogLevel::Debug5)
        << "filters.python " << *m_script << " processing " << view->size()
        << " points." << std::endl;
    m_pythonMethod->resetArguments();
    m_pythonMethod->begin(*view, m_totalMetadata);

    m_pythonMethod->setKWargs(m_pdalargs);
    m_pythonMethod->execute();

    PointViewSet viewSet;

    m_pythonMethod->end(*view, getMetadata());
    viewSet.insert(view);

    return viewSet;
}

void CmapFilter::done(PointTableRef table)
{
    static_cast<plang::Environment*>(plang::Environment::get())->reset_stdout();
    delete m_pythonMethod;
    delete m_script;
}

} // namespace pdal
