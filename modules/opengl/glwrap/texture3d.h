/**********************************************************************
 * Copyright (C) 2012-2013 Scientific Visualization Group - Link�ping University
 * All Rights Reserved.
 * 
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * No part of this software may be reproduced or transmitted in any
 * form or by any means including photocopying or recording without
 * written permission of the copyright owner.
 *
 * Primary author : Timo Ropinski
 *
 **********************************************************************/

#ifndef IVW_TEXTURE3D_H
#define IVW_TEXTURE3D_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <stdio.h>
#include <fstream>

namespace inviwo {

class IVW_MODULE_OPENGL_API Texture3D {

public:
    Texture3D(uvec3 dimensions, GLint format, GLint internalformat, GLenum dataType, GLenum filtering);
    virtual ~Texture3D();

    unsigned int getID() const { return id_; }

    void bind() const;
    void unbind() const;
    void upload(const void* data);
    void download(void* data) const;

    const uvec3& getDimension() const { return dimensions_; }

private:
    uvec3 dimensions_;
    GLenum format_;
    GLenum internalformat_;
    GLenum dataType_;
    GLenum filtering_;

    GLuint id_;
};

} // namespace

#endif // IVW_TEXTURE3D_H
