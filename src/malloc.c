/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be
used in advertising or publicity pertaining to distribution
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "xkballoc.h"
#include "xkbcommon/xkbcommon.h"
#include "XKBcommonint.h"

int
XkbcAllocClientMap(struct xkb_desc * xkb, unsigned which, unsigned nTotalTypes)
{
    int i;
    struct xkb_client_map * map;

    if (!xkb || ((nTotalTypes > 0) && (nTotalTypes < XkbNumRequiredTypes)))
        return BadValue;

    if ((which & XkbKeySymsMask) && !xkb_keymap_keycode_range_is_legal(xkb)) {
#ifdef DEBUG
        fprintf(stderr, "bad keycode (%d,%d) in XkbAllocClientMap\n",
                xkb->min_key_code, xkb->max_key_code);
#endif
        return BadValue;
    }

    if (!xkb->map) {
        map = _XkbTypedCalloc(1, struct xkb_client_map);
        if (!map)
            return BadAlloc;
        xkb->map = map;
    }
    else
        map = xkb->map;

    if ((which & XkbKeyTypesMask) && (nTotalTypes > 0)) {
        if (!map->types) {
            map->types = _XkbTypedCalloc(nTotalTypes, struct xkb_key_type);
            if (!map->types)
                return BadAlloc;

            map->num_types = 0;
            map->size_types = nTotalTypes;
        }
        else if (map->size_types < nTotalTypes) {
            struct xkb_key_type *prev_types = map->types;

            map->types = _XkbTypedRealloc(map->types, nTotalTypes,
                                          struct xkb_key_type);
            if (!map->types) {
                free(prev_types);
                map->num_types = map->size_types = 0;
                return BadAlloc;
            }

            map->size_types = nTotalTypes;
            bzero(&map->types[map->num_types],
                  (map->size_types - map->num_types) * sizeof(struct xkb_key_type));
        }
    }

    if (which & XkbKeySymsMask) {
        int nKeys = XkbNumKeys(xkb);

        if (!map->syms) {
            map->size_syms = (nKeys * 15) / 10;
            map->syms = _XkbTypedCalloc(map->size_syms, uint32_t);
            if (!map->syms) {
                map->size_syms = 0;
                return BadAlloc;
            }
            map->num_syms = 1;
            map->syms[0] = NoSymbol;
        }

        if (!map->key_sym_map) {
            i = xkb->max_key_code + 1;
            map->key_sym_map = _XkbTypedCalloc(i, struct xkb_sym_map);
            if (!map->key_sym_map)
                return BadAlloc;
        }
    }

    if (which & XkbModifierMapMask) {
        if (!xkb_keymap_keycode_range_is_legal(xkb))
            return BadMatch;

        if (!map->modmap) {
            i = xkb->max_key_code + 1;
            map->modmap = _XkbTypedCalloc(i, unsigned char);
            if (!map->modmap)
                return BadAlloc;
        }
    }

    return Success;
}

int
XkbcAllocServerMap(struct xkb_desc * xkb, unsigned which, unsigned nNewActions)
{
    int i;
    struct xkb_server_map * map;

    if (!xkb)
        return BadMatch;

    if (!xkb->server) {
        map = _XkbTypedCalloc(1, struct xkb_server_map);
        if (!map)
            return BadAlloc;

        for (i = 0; i < XkbNumVirtualMods; i++)
            map->vmods[i] = XkbNoModifierMask;

        xkb->server = map;
    }
    else
        map = xkb->server;

    if (!which)
        return Success;

    if (!xkb_keymap_keycode_range_is_legal(xkb))
        return BadMatch;

    if (which & XkbExplicitComponentsMask) {
        if (!map->explicit) {
            i = xkb->max_key_code + 1;
            map->explicit = _XkbTypedCalloc(i, unsigned char);
            if (!map->explicit)
                return BadAlloc;
        }
    }

    if (which&XkbKeyActionsMask) {
        if (nNewActions < 1)
            nNewActions = 1;

        if (!map->acts) {
            map->acts = _XkbTypedCalloc(nNewActions + 1, union xkb_action);
            if (!map->acts)
                return BadAlloc;
            map->num_acts = 1;
            map->size_acts = nNewActions + 1;
        }
        else if ((map->size_acts - map->num_acts) < nNewActions) {
            unsigned need;
            union xkb_action *prev_acts = map->acts;

            need = map->num_acts + nNewActions;
            map->acts = _XkbTypedRealloc(map->acts, need, union xkb_action);
            if (!map->acts) {
                free(prev_acts);
                map->num_acts = map->size_acts = 0;
                return BadAlloc;
            }

            map->size_acts = need;
            bzero(&map->acts[map->num_acts],
                  (map->size_acts - map->num_acts) * sizeof(union xkb_action));
        }

        if (!map->key_acts) {
            i = xkb->max_key_code + 1;
            map->key_acts = _XkbTypedCalloc(i, unsigned short);
            if (!map->key_acts)
                return BadAlloc;
        }
    }

    if (which & XkbKeyBehaviorsMask) {
        if (!map->behaviors) {
            i = xkb->max_key_code + 1;
            map->behaviors = _XkbTypedCalloc(i, struct xkb_behavior);
            if (!map->behaviors)
                return BadAlloc;
        }
    }

    if (which & XkbVirtualModMapMask) {
        if (!map->vmodmap) {
            i = xkb->max_key_code + 1;
            map->vmodmap = _XkbTypedCalloc(i, uint32_t);
            if (!map->vmodmap)
                return BadAlloc;
        }
    }

    return Success;
}

int
XkbcCopyKeyType(struct xkb_key_type * from, struct xkb_key_type * into)
{
    if (!from || !into)
        return BadMatch;

    free(into->map);
    into->map = NULL;
    free(into->preserve);
    into->preserve= NULL;
    free(into->level_names);
    into->level_names = NULL;

    *into = *from;

    if (from->map && (into->map_count > 0)) {
        into->map = _XkbTypedCalloc(into->map_count, struct xkb_kt_map_entry);
        if (!into->map)
            return BadAlloc;
        memcpy(into->map, from->map,
               into->map_count * sizeof(struct xkb_kt_map_entry));
    }

    if (from->preserve && (into->map_count > 0)) {
        into->preserve = _XkbTypedCalloc(into->map_count, struct xkb_mods);
        if (!into->preserve)
            return BadAlloc;
        memcpy(into->preserve, from->preserve,
               into->map_count * sizeof(struct xkb_mods));
    }

    if (from->level_names && (into->num_levels > 0)) {
        into->level_names = _XkbTypedCalloc(into->num_levels, uint32_t);
        if (!into->level_names)
            return BadAlloc;
        memcpy(into->level_names, from->level_names,
               into->num_levels * sizeof(uint32_t));
    }

    return Success;
}

uint32_t *
XkbcResizeKeySyms(struct xkb_desc * xkb, xkb_keycode_t key,
                  unsigned int needed)
{
    uint32_t i, nSyms, nKeySyms;
    uint32_t nOldSyms;
    uint32_t *newSyms;

    if (needed == 0) {
        xkb->map->key_sym_map[key].offset = 0;
        return xkb->map->syms;
    }

    nOldSyms = XkbKeyNumSyms(xkb, key);
    if (nOldSyms >= needed)
        return XkbKeySymsPtr(xkb, key);

    if (xkb->map->size_syms - xkb->map->num_syms >= needed) {
        if (nOldSyms > 0)
            memcpy(&xkb->map->syms[xkb->map->num_syms],
                   XkbKeySymsPtr(xkb, key), nOldSyms * sizeof(uint32_t));

        if ((needed - nOldSyms) > 0)
            bzero(&xkb->map->syms[xkb->map->num_syms + XkbKeyNumSyms(xkb, key)],
                  (needed - nOldSyms) * sizeof(uint32_t));

        xkb->map->key_sym_map[key].offset = xkb->map->num_syms;
        xkb->map->num_syms += needed;

        return &xkb->map->syms[xkb->map->key_sym_map[key].offset];
    }

    xkb->map->size_syms += (needed > 32 ? needed : 32);
    newSyms = _XkbTypedCalloc(xkb->map->size_syms, uint32_t);
    if (!newSyms)
        return NULL;

    newSyms[0] = NoSymbol;
    nSyms = 1;
    for (i = xkb->min_key_code; i <= xkb->max_key_code; i++) {
        int nCopy;

        nCopy = nKeySyms = XkbKeyNumSyms(xkb, i);
        if ((nKeySyms == 0) && (i != key))
            continue;

        if (i == key)
            nKeySyms = needed;
        if (nCopy != 0)
           memcpy(&newSyms[nSyms], XkbKeySymsPtr(xkb, i),
                  nCopy * sizeof(uint32_t));
        if (nKeySyms > nCopy)
            bzero(&newSyms[nSyms+nCopy], (nKeySyms - nCopy) * sizeof(uint32_t));

        xkb->map->key_sym_map[i].offset = nSyms;
        nSyms += nKeySyms;
    }

    free(xkb->map->syms);
    xkb->map->syms = newSyms;
    xkb->map->num_syms = nSyms;

    return &xkb->map->syms[xkb->map->key_sym_map[key].offset];
}

union xkb_action *
XkbcResizeKeyActions(struct xkb_desc * xkb, xkb_keycode_t key, int needed)
{
    xkb_keycode_t i, nActs;
    union xkb_action *newActs;

    if (needed == 0) {
        xkb->server->key_acts[key] = 0;
        return NULL;
    }

    if (XkbKeyHasActions(xkb, key) &&
        (XkbKeyNumSyms(xkb, key) >= (unsigned)needed))
        return XkbKeyActionsPtr(xkb, key);

    if (xkb->server->size_acts - xkb->server->num_acts >= (unsigned)needed) {
        xkb->server->key_acts[key] = xkb->server->num_acts;
        xkb->server->num_acts += needed;

        return &xkb->server->acts[xkb->server->key_acts[key]];
    }

    xkb->server->size_acts = xkb->server->num_acts + needed + 8;
    newActs = _XkbTypedCalloc(xkb->server->size_acts, union xkb_action);
    if (!newActs)
        return NULL;
    newActs[0].type = XkbSA_NoAction;
    nActs = 1;

    for (i = xkb->min_key_code; i <= xkb->max_key_code; i++) {
        xkb_keycode_t nKeyActs, nCopy;

        if ((xkb->server->key_acts[i] == 0) && (i != key))
            continue;

        nCopy = nKeyActs = XkbKeyNumActions(xkb, i);
        if (i == key) {
            nKeyActs= needed;
            if (needed < nCopy)
                nCopy = needed;
        }

        if (nCopy > 0)
            memcpy(&newActs[nActs], XkbKeyActionsPtr(xkb, i),
                   nCopy * sizeof(union xkb_action));
        if (nCopy < nKeyActs)
            bzero(&newActs[nActs + nCopy],
                  (nKeyActs - nCopy) * sizeof(union xkb_action));

        xkb->server->key_acts[i] = nActs;
        nActs += nKeyActs;
    }

    free(xkb->server->acts);
    xkb->server->acts = newActs;
    xkb->server->num_acts = nActs;

    return &xkb->server->acts[xkb->server->key_acts[key]];
}

void
XkbcFreeClientMap(struct xkb_desc * xkb, unsigned what, Bool freeMap)
{
    struct xkb_client_map * map;

    if (!xkb || !xkb->map)
        return;

    if (freeMap)
        what = XkbAllClientInfoMask;
    map = xkb->map;

    if (what & XkbKeyTypesMask) {
        int i;
        struct xkb_key_type * type;

        for (i = 0, type = map->types; i < map->num_types && type; i++, type++) {
            free(type->map);
            type->map = NULL;
            free(type->preserve);
            type->preserve = NULL;
            type->map_count = 0;
            free(type->level_names);
            type->level_names = NULL;
        }
        free(map->types);
        map->num_types = map->size_types = 0;
        map->types = NULL;
    }

    if (what & XkbKeySymsMask) {
        free(map->key_sym_map);
        map->key_sym_map = NULL;
        free(map->syms);
        map->size_syms = map->num_syms = 0;
        map->syms = NULL;
    }

    if (what & XkbModifierMapMask) {
        free(map->modmap);
        map->modmap = NULL;
    }

    if (freeMap) {
        free(xkb->map);
        xkb->map = NULL;
    }
}

void
XkbcFreeServerMap(struct xkb_desc * xkb, unsigned what, Bool freeMap)
{
    struct xkb_server_map * map;

    if (!xkb || !xkb->server)
        return;

    if (freeMap)
        what = XkbAllServerInfoMask;
    map = xkb->server;

    if (what & XkbExplicitComponentsMask) {
        free(map->explicit);
        map->explicit = NULL;
    }

    if (what & XkbKeyActionsMask) {
        free(map->key_acts);
        map->key_acts = NULL;
        free(map->acts);
        map->num_acts = map->size_acts = 0;
        map->acts = NULL;
    }

    if (what & XkbKeyBehaviorsMask) {
        free(map->behaviors);
        map->behaviors = NULL;
    }

    if (what & XkbVirtualModMapMask) {
        free(map->vmodmap);
        map->vmodmap = NULL;
    }

    if (freeMap) {
        free(xkb->server);
        xkb->server = NULL;
    }
}
