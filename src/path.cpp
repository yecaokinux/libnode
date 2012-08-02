// Copyright (c) 2012 Plenluno All rights reserved.

#include <assert.h>
#include <unistd.h>
#include <libj/string_buffer.h>

#include "libnode/path.h"

namespace libj {
namespace node {
namespace path {

String::CPtr normalize(String::CPtr path) {
    static const String::CPtr empty = String::create();
    static const String::CPtr current = String::create(".");
    static const String::CPtr parent = String::create("..");

    if (!path) return empty;

    Boolean absolute = false;
    Boolean endsWithSep = false;
    JsArray::Ptr dirs = JsArray::create();
    Size len = path->length();
    for (Size i = 0; i < len;) {
        Size idx = path->indexOf('/', i);
        if (idx == 0) {
            absolute = true;
        } else if (idx != i) {
            String::CPtr dir;
            if (idx == NO_POS) {
                dir = path->substring(i);
            } else {
                dir = path->substring(i, idx);
            }
            if (dir->compareTo(parent) == 0) {
                Size numDirs = dirs->size();
                if (numDirs > 0 &&
                    dirs->get(numDirs - 1).compareTo(parent) != 0) {
                    dirs->remove(numDirs - 1);
                } else {
                    dirs->add(dir);
                }
            } else if (dir->compareTo(current) != 0) {
                dirs->add(dir);
            }
        }

        if (idx == NO_POS) {
            endsWithSep = false;
            i = len;
        } else {
            endsWithSep = true;
            i = idx + 1;
        }
    }

    StringBuffer::Ptr normal = StringBuffer::create();
    if (absolute)
        normal->append(sep());
    Size numDirs = dirs->size();
    for (Size i = 0; i < numDirs; i++) {
        if (i) normal->append(sep());
        normal->append(dirs->get(i));
    }
    if (numDirs > 0 && endsWithSep)
        normal->append(sep());
    if (normal->length() == 0) {
        return current;
    } else {
        return normal->toString();
    }
}

String::CPtr join(JsArray::CPtr paths) {
    static const String::CPtr current = String::create(".");

    if (!paths) return current;

    StringBuffer::Ptr joined = StringBuffer::create();
    Size len = paths->length();
    Boolean first = true;
    for (Size i = 0; i < len; i++) {
        String::CPtr path = toCPtr<String>(paths->get(i));
        if (path) {
            if (first) {
                first = false;
            } else {
                joined->append(sep());
            }
            joined->append(path);
        }
    }
    return normalize(joined->toString());
}


static String::CPtr getCwd() {
    const Size kMax = 8192;
    char dir[kMax];
    getcwd(dir, kMax);
    return String::create(dir);
}

String::CPtr resolve(JsArray::CPtr paths) {
    if (!paths) return getCwd();

    StringBuffer::Ptr resolved = StringBuffer::create();
    resolved->append(getCwd());
    Size len = paths->length();
    for (Size i = 0; i < len; i++) {
        String::CPtr path = toCPtr<String>(paths->get(i));
        if (path) {
            if (path->startsWith(sep())) {
                resolved = StringBuffer::create();
            } else {
                resolved->append(sep());
            }
            resolved->append(path);
        }
    }
    return normalize(resolved->toString());
}

String::CPtr relative(String::CPtr from, String::CPtr to) {
    // TODO(plenluno): implement
    return String::create();
}

String::CPtr dirname(String::CPtr path) {
    static const String::CPtr current = String::create(".");

    if (!path) return current;

    String::CPtr base = basename(path);
    Size baseLen = base->length();
    Size pathLen = path->length();
    if (baseLen == pathLen) {
        return current;
    } else {
        Size sepPos = pathLen - baseLen - 1;
        assert(path->charAt(sepPos) == '/');
        if (sepPos) {
            return path->substring(0, sepPos);
        } else {  // path[0] is '/'
            return sep();
        }
    }
}

String::CPtr basename(String::CPtr path) {
    static const String::CPtr empty = String::create();
    static const String::CPtr doubleSep = sep()->concat(sep());

    if (!path) return empty;
    if (path->compareTo(sep()) == 0) return empty;
    if (path->compareTo(doubleSep) == 0) return empty;

    Size lastIndex;
    Size len = path->length();
    if (path->endsWith(sep())) {
        Size from = len - sep()->length() - 1;
        lastIndex = path->lastIndexOf(sep(), from);
    } else {
        lastIndex = path->lastIndexOf(sep());
    }
    String::CPtr base;
    if (lastIndex == NO_POS) {
        base = path;
    } else {
        base = path->substring(lastIndex + 1);
    }
    return base;
}

String::CPtr extname(String::CPtr path) {
    static const String::CPtr empty = String::create();

    if (!path) return empty;

    String::CPtr base = basename(path);
    if (base->endsWith(sep())) return empty;

    Size lastIndex = NO_POS;
    if (base->length() > 1)
        lastIndex = base->lastIndexOf('.');
    String::CPtr ext;
    if (lastIndex == NO_POS) {
        ext = empty;
    } else {
        ext = base->substring(lastIndex);
    }
    return ext;
}

String::CPtr sep() {
    static const String::CPtr separator = String::create("/");
    return separator;
}

}  // namespace path
}  // namespace node
}  // namespace libj
