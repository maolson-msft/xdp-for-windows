//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#pragma once

#include <xdprefcount.h>

#include "rss.h"
#include "send.h"

#define GENERIC_DATAPATH_RESTART_TIMEOUT_MS 1000

// TODO: break circular dependency Filter->Generic->Filter.
//       Add an interface context when creating an XDP binding?
//       Or just make this opaque context?
typedef struct _XDP_LWF_FILTER XDP_LWF_FILTER;

typedef struct _XDP_LWF_DATAPATH_BYPASS {
    BOOLEAN Inserted;           // LWF handlers are inserted on the NDIS data path.
    INT64 ReferenceCount;       // Number of data path clients.
    KEVENT ReadyEvent;          // Set when the inserted data path is restarted.
    XDP_TIMER *DelayDetachTimer;
} XDP_LWF_DATAPATH_BYPASS;

typedef struct _XDP_LWF_GENERIC {
    XDP_LWF_FILTER *Filter;
    NDIS_HANDLE NdisFilterHandle;
    NET_IFINDEX IfIndex;

    XDP_CAPABILITIES Capabilities;
    XDP_CAPABILITIES_INTERNAL InternalCapabilities;

    EX_PUSH_LOCK Lock;
    XDP_REGISTRATION_HANDLE Registration;
    XDPIF_INTERFACE_HANDLE XdpIfInterfaceHandle;
    KEVENT InterfaceRemovedEvent;
    XDP_REFERENCE_COUNT ReferenceCount;
    KEVENT CleanupEvent;

    XDP_LWF_GENERIC_RSS Rss;

    struct {
        XDP_LWF_DATAPATH_BYPASS Datapath;
    } Rx;

    struct {
        XDP_LWF_DATAPATH_BYPASS Datapath;
        LIST_ENTRY Queues;
        UINT32 Mtu;
    } Tx;
} XDP_LWF_GENERIC;

XDP_LWF_GENERIC *
XdpGenericFromFilterContext(
    _In_ NDIS_HANDLE FilterModuleContext
    );

NTSTATUS
XdpGenericAttachInterface(
    _Inout_ XDP_LWF_GENERIC *Generic,
    _In_ XDP_LWF_FILTER *Filter,
    _In_ NDIS_HANDLE NdisFilterHandle,
    _In_ NET_IFINDEX IfIndex,
    _Out_ XDP_ADD_INTERFACE *AddIf
    );

VOID
XdpGenericDetachInterface(
    _In_ XDP_LWF_GENERIC *Generic
    );

VOID
XdpGenericPause(
    _In_ XDP_LWF_GENERIC *Generic
    );

VOID
XdpGenericRestart(
    _In_ XDP_LWF_GENERIC *Generic,
    _In_ NDIS_FILTER_RESTART_PARAMETERS *RestartParameters
    );

NDIS_STATUS
XdpGenericFilterSetOptions(
    _In_ XDP_LWF_GENERIC *Generic
    );

NDIS_STATUS
XdpGenericInspectOidRequest(
    _In_ XDP_LWF_GENERIC *Generic,
    _In_ NDIS_OID_REQUEST *Request
    );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
XdpGenericRequestRestart(
    _In_ XDP_LWF_GENERIC *Generic
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
_Requires_lock_held_(&Generic->Lock)
VOID
XdpGenericReferenceDatapath(
    _In_ XDP_LWF_GENERIC *Generic,
    _In_ XDP_LWF_DATAPATH_BYPASS *Datapath,
    _Out_ BOOLEAN *NeedRestart
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
_Requires_lock_held_(&Generic->Lock)
VOID
XdpGenericDereferenceDatapath(
    _In_ XDP_LWF_GENERIC *Generic,
    _In_ XDP_LWF_DATAPATH_BYPASS *Datapath,
    _Out_ BOOLEAN *NeedRestart
    );

NTSTATUS
XdpGenericStart(
    VOID
    );

VOID
XdpGenericStop(
    VOID
    );
