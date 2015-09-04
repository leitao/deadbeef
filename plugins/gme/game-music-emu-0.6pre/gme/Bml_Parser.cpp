#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Bml_Parser.h"

Bml_Parser::Bml_Parser() {
    nodes = NULL;
    tail = NULL;
}

Bml_Parser::~Bml_Parser() {
    clearDocument();
}

void Bml_Parser::clearDocument ()
{
    while (nodes) {
        if (nodes->key) {
            free (nodes->key);
        }
        if (nodes->value) {
            free (nodes->value);
        }
        nodes = nodes->next;
    }
    tail = NULL;
}

void Bml_Parser::addNode (const char *path, const char *value)
{
    Bml_Node *node = new Bml_Node;
    memset (node, 0, sizeof (Bml_Node));
    node->key = strdup (path);
    if (value) {
        node->value = strdup (value);
    }
    if (tail) {
        tail->next = node;
    }
    else {
        nodes = node;
    }
    tail = node;
}

void Bml_Parser::parseDocument( const char * source, size_t max_length )
{
    clearDocument();

    const char *p = source;
    const char *end = p + max_length;

    char path[200] = "";

    int indents[100];
    int num_indents = 0;

    while (p < end) {
        int indent = 0;
        while (p < end && *p == 0x20) {
            indent++;
            p++;
        }
        while (num_indents > 0 && indents[num_indents-1] >= indent) {
            char *colon = strrchr (path, ':');
            if (colon) {
                *colon = 0;
            }
            num_indents--;
        }
        indents[num_indents++] = indent;

        const char *e = p;
        while (e < end && *e != '\n') {
            e++;
        }
        if (e == p || indent == 0) {
            path[0] = 0;
        }
        if (e != p) {
            char name[e-p+1];
            memcpy (name, p, e-p);
            name[e-p] = 0;

            char *colon = strrchr (name, ':');
            if (colon) {
                *colon = 0;
            }
            if (indent != 0) {
                strcat (path, ":");
            }

            strcat (path, name);
            if (colon) {
                addNode (path, colon+1);
                printf ("%s = %s\n", path, colon+1);
            }
            else {
                addNode (path, NULL);
                printf ("---- %s\n", path);
            }
        }
        p = e;
        p++;
    }
}

Bml_Parser::Bml_Node *Bml_Parser::walkToNode (const char *_path) const
{
    Bml_Node *node = nodes;

    char *path = strdup(_path);

    // split on : or [
    char *p = path;
    while (*p) {
        if (*p == '[') {
            int n = atoi(p+1) + 1;
            char *e = p;
            while (*e && *e != ':') {
                e++;
            }
            memmove (p, e, strlen (e)+1);
            // find n'th node starting with [p..e]
            for (; n && node; node = node->next) {
                size_t l = p-path;
                if (!strncmp (node->key, path, l) && !node->key[l]) {
                    n--;
                }
            }
        }
        p++;
    }
    for (; node; node = node->next) {
        if (!strcmp (node->key, path)) {
            free (path);
            return node;
        }
    }
    free (path);
    return NULL;
}

const char * Bml_Parser::enumValue(const char *path) const
{
    const Bml_Node *node = walkToNode (path);
    if (node) {
        return node->value;
    }
    return NULL;
}

void Bml_Parser::setValue(const char *path, const char *value)
{
    Bml_Node *node = walkToNode (path);
    if (node) {
        free (node->value);
        node->value = strdup (value);
        return;
    }
    addNode(path, value);
}

void Bml_Parser::setValue(const char *path, long value)
{
    char str[15];
    snprintf (str, sizeof (str), "%ld", value);
    setValue( path, str );
}

void Bml_Parser::serialize(char *buffer, int size) const
{
//    serialize(buffer, size, &document, 0);
}

#if 0
void Bml_Parser::serialize(char *buffer, int size, Bml_Node const* node, unsigned int indent) const
{
    size_t l = 0;
#define APPEND(x) l = strlen(x); if (l > size) return; strcat (buffer, x); buffer += l; size -= l;

    for (unsigned i = 1; i < indent; ++i) {
        APPEND("  ");
    }

    if ( indent )
    {
        APPEND(node->getName());
        if (node->getValue() && strlen(node->getValue())) {
            APPEND(":");
            APPEND(node->getValue());
        }
        APPEND("\n");
    }

    for (unsigned i = 0, j = node->getChildCount(); i < j; ++i)
    {
        Bml_Node const& child = node->getChild(i);
        if ( (!child.getValue() || !strlen(child.getValue())) && !child.getChildCount() )
            continue;
        serialize( buffer, size, &child, indent + 1 );
        if ( indent == 0 ) APPEND("\n");
    }
}
#endif