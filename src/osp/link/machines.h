/**
 * Open Space Program
 * Copyright © 2019-2022 Open Space Program Project
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
#pragma once

#include "../global_id.h"

#include <longeron/containers/intarray_multimap.hpp>
#include <longeron/containers/bit_view.hpp>
#include <longeron/id_management/registry_stl.hpp>

#include <Corrade/Containers/ArrayView.h>

#include <vector>

namespace osp::link
{

using BitVector_t = lgrn::BitView< std::vector<uint64_t> >;

using MachTypeId    = uint16_t;
using MachAnyId     = uint32_t;
using MachLocalId   = uint32_t;

using NodeTypeId    = uint16_t;
using NodeId        = uint32_t;

using PortId        = uint16_t;
using JunctionId    = uint16_t;
using JuncCustom    = uint16_t;

using MachTypeReg_t = GlobalIdReg<MachLocalId>;
using NodeTypeReg_t = GlobalIdReg<NodeTypeId>;

extern NodeTypeId const gc_ntNumber;

/**
 * @brief Keeps track of Machines of a certain type that exists
 */
struct PerMachType
{
    lgrn::IdRegistryStl<MachLocalId>    m_localIds;
    std::vector<MachAnyId>              m_localToAny;
};

/**
 * @brief Keeps track of all Machines that exist and what type they are
 */
struct Machines
{
    lgrn::IdRegistryStl<MachAnyId>      m_ids;

    std::vector<MachTypeId>             m_machTypes;
    std::vector<MachLocalId>            m_machToLocal;

    std::vector<PerMachType>            m_perType;
};

struct UpdateMach
{
    BitVector_t m_localDirty;
};

// [MachTypeId].m_locaDirty[MachLocalId]
using UpdMachPerType_t = std::vector<UpdateMach>;

struct Junction
{
    MachLocalId     m_local{lgrn::id_null<MachLocalId>()};
    MachTypeId      m_type{lgrn::id_null<MachTypeId>()};
    JuncCustom      m_custom{0};
};

/**
 * @brief Connects Machines together with intermediate Nodes
 */
struct Nodes
{
    // reminder: IntArrayMultiMap is kind of like an
    //           std::vector< std::vector<...> > but more memory efficient
    using NodeToMach_t = lgrn::IntArrayMultiMap<NodeId, Junction>;
    using MachToNode_t = lgrn::IntArrayMultiMap<MachAnyId, NodeId>;

    lgrn::IdRegistryStl<NodeId>         m_nodeIds;

    // Node-to-Machine connections
    // [NodeId][JunctionIndex] -> Junction (type, MachLocalId, custom int)
    NodeToMach_t                        m_nodeToMach;

    // Corresponding Machine-to-Node connections
    // [MachAnyId][PortIndex] -> NodeId
    MachToNode_t                        m_machToNode;
};

struct PortEntry
{
    NodeTypeId  m_type;
    PortId      m_port;
    JuncCustom  m_custom;
};

inline NodeId connected_node(lgrn::Span<NodeId const> portSpan, PortId port) noexcept
{
    return (portSpan.size() > port) ? portSpan[port] : lgrn::id_null<NodeId>();
}

void copy_machines(
        Machines const &rSrc,
        Machines &rDst,
        Corrade::Containers::ArrayView<MachAnyId> remapMach);

void copy_nodes(
        Nodes const &rSrcNodes,
        Machines const &rSrcMach,
        Corrade::Containers::ArrayView<MachAnyId const> remapMach,
        Nodes &rDstNodes,
        Machines &rDstMach,
        Corrade::Containers::ArrayView<NodeId> remapNode);


} // namespace osp::wire
