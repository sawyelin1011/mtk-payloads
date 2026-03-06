/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_XML_XML_H
#define DA_XML_XML_H

const char *get_node_text(void *tree, const char *path);
bool mxmlCheckNodes(void *tree, ...);

// TODO: Use mxmlDelete instead of free
#define MXMLDELETE(tree)        \
    do {                        \
        if (tree) {             \
            free(tree);         \
            tree = NULL;        \
        }                       \
    } while (0)

#define MXML_LOAD(tree, xml, callback, ...)                 \
    do {                                                    \
        tree = mxmlLoadString(NULL, xml, callback);         \
        if (!tree || !mxmlCheckNodes(tree, __VA_ARGS__)) {  \
            MXMLDELETE(tree);                               \
            return STATUS_ERR;                              \
        }                                                   \
    } while (0)


#endif //DA_XML_XML_H
