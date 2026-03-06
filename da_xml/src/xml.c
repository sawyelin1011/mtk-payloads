/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdarg.h>
#include <protocol_functions.h>

#define MXML_TYPE_ELEMENT  1
#define MXML_TYPE_TEXT     2
#define MXML_TYPE_OPAQUE   3

/*
 * This is required because MediaTek managed to break ABI compatibility
 * even in their own mxml implementation across DA versions. Some return
 * strings directly, others return node pointers. Classic MediaTek...
 */
const char *get_node_text(void *tree, const char *path)
{
    void *result = mxmlGetNodeText(tree, path);
    if (!result)
        return NULL;

    uintptr_t *n = (uintptr_t *)result;

    /* Text/opaque node: type 2|3, no children, has value */
    if ((n[0] == MXML_TYPE_TEXT || n[0] == MXML_TYPE_OPAQUE) &&
        n[4] == 0 && n[5] == 0 && n[6] != 0)
        return (const char *)n[6];

    /* Element node with text child */
    if (n[0] == MXML_TYPE_ELEMENT && n[4] != 0) {
        uintptr_t *c = (uintptr_t *)n[4];
        if ((c[0] == MXML_TYPE_TEXT || c[0] == MXML_TYPE_OPAQUE) &&
            c[4] == 0 && c[5] == 0 && c[6] != 0)
            return (const char *)c[6];
    }

    /* Assume direct string */
    return (const char *)result;
}

bool mxmlCheckNodes(void* tree, ...) {
    va_list args;
    const char *name;
    va_start(args, tree);
    while ((name = va_arg(args, const char *))) {
        if (!get_node_text(tree, name)) {
            va_end(args);
            return false;
        }
    }
    va_end(args);
    return true;
}
