////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2009
//
//    This file is part of Scorched3D.
//
//    Scorched3D is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Scorched3D is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Scorched3D; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

#include <GLEXT/GLTextureReference.h>
#include <GLEXT/GLTextureStore.h>
#include <image/ImageFactory.h>
#include <GLEXT/GLTexture.h>

GLTextureReference::GLTextureReference(const ImageID &imageId, unsigned texState) :
	imageId_(imageId),
	texState_(texState),
	texture_(0)
{
	GLTextureStore::instance()->addTextureReference(*this);
}

GLTextureReference::GLTextureReference() :
	texture_(0),
	texState_(eMipMap)
{
	GLTextureStore::instance()->addTextureReference(*this);
}

GLTextureReference::~GLTextureReference()
{
	GLTextureStore::instance()->removeTextureReference(*this);
	reset();
}

void GLTextureReference::setImageID(const ImageID &imageId, unsigned texState) 
{
	reset();
	imageId_ = imageId;
	texState_ = texState;
}

void GLTextureReference::reset()
{
	delete texture_;
	texture_ = 0;
}

GLTexture *GLTextureReference::getTexture()
{
	if (!texture_)
	{
		Image image = ImageFactory::loadImageID(imageId_);
		texture_ = new GLTexture();
		texture_->create(image, texState_ & eMipMap);
		if (texState_ & eTextureClamped)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
	}
	return texture_;
}

void GLTextureReference::draw(bool force)
{
	getTexture()->draw(force);
}
