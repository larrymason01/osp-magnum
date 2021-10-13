/**
 * Open Space Program
 * Copyright © 2019-2021 Open Space Program Project
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "SubdivSkeleton.h"

#include <stdexcept>

using namespace planeta;

SkTriGroupId SubdivTriangleSkeleton::tri_subdiv(SkTriId triId,
                                                std::array<SkVrtxId, 3> vrtxMid)
{
    SkeletonTriangle& rTri = tri_at(triId);

    if ( ! rTri.m_children.has_value())
    {
        throw std::runtime_error("SkeletonTriangle is already subdivided");
    }

    SkTriGroup const parentGroup = m_triData[size_t(tri_group_id(triId))];


    //std::array<std::array<SkVrtxId, 3>, 4>
    SkTriGroupId const groupId = tri_group_create(parentGroup.m_depth, triId,
    {{
        {},
        {},
        {},
        {}
    }});

    rTri.m_children = groupId;

    return groupId;
}
