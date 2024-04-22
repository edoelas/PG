#include "app.h"
#include "utils.h"
#include "texture.h"
#include "log.h"

using PGUPV::Texture;
using PGUPV::App;
using std::string;

Texture::Texture(GLenum texture_type, GLenum minfilter, GLenum magfilter,
	GLenum wrap_s, GLenum wrap_t, GLenum wrap_r,
	const glm::vec4& bordercolor)
	: BindableTexture(texture_type), _minfilter(minfilter),
	_magfilter(magfilter), _wrap_s(wrap_s), _wrap_t(wrap_t), _wrap_r(wrap_r),
	_compareMode(GL_NONE), _compareFunc(GL_LEQUAL), _internalFormat(GL_NONE),
	_bordercolor(bordercolor) {}

void Texture::setMinFilter(GLenum filter) {

	setTexParam(GL_TEXTURE_MIN_FILTER, filter);
	_minfilter = filter;
}

void Texture::setMagFilter(GLenum filter) {

	setTexParam(GL_TEXTURE_MAG_FILTER, filter);
	_magfilter = filter;
}

void Texture::setWrapS(GLenum wrap) {

	setTexParam(GL_TEXTURE_WRAP_S, wrap);
	_wrap_s = wrap;
}

void Texture::setWrapT(GLenum wrap) {
	setTexParam(GL_TEXTURE_WRAP_T, wrap);
	_wrap_t = wrap;
}

void Texture::setWrapR(GLenum wrap) {
	setTexParam(GL_TEXTURE_WRAP_R, wrap);
	_wrap_r = wrap;
}

void Texture::setCompareMode(GLenum compareMode) {
	setTexParam(GL_TEXTURE_COMPARE_MODE, compareMode);
	_compareMode = compareMode;
}

void Texture::setCompareFunc(GLenum compareFunc) {
	setTexParam(GL_TEXTURE_COMPARE_FUNC, compareFunc);
	_compareFunc = compareFunc;
}

void Texture::setBorderColor(const glm::vec4& color) {
	setTexParam(GL_TEXTURE_BORDER_COLOR, &color[0]);
	_bordercolor = color;
}

GLint activateScratchTextureUnit() {
	GLint texunit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &texunit);
	if (texunit != (GL_TEXTURE0 + App::getScratchUnitTextureNumber()))
		glActiveTexture(GL_TEXTURE0 + App::getScratchUnitTextureNumber());
	return texunit;
}

void deactivateScratchTextureUnit(GLint prevTextureUnit) {
	if (prevTextureUnit != (GL_TEXTURE0 + App::getScratchUnitTextureNumber()))
		glActiveTexture(GL_TEXTURE0 + App::getScratchUnitTextureNumber());
}

Texture::TextureLevelInfo Texture::getLevelInfo(GLint level) const
{
	TextureLevelInfo result;
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_WIDTH, &result.width);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_HEIGHT, &result.height);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_DEPTH, &result.depth);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_INTERNAL_FORMAT, reinterpret_cast<GLint*>(&result.internalFormat));
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_RED_SIZE, &result.redSize);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_GREEN_SIZE, &result.greenSize);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_BLUE_SIZE, &result.blueSize);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_ALPHA_SIZE, &result.alphaSize);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_DEPTH_SIZE, &result.depthSize);
	GLint temp;
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_COMPRESSED, &temp);
	result.compressed = temp != 0;
	if (result.compressed) {
		glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &result.compressedImageSize);
	}
	else {
		result.compressedImageSize = 0;
	}
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_BUFFER_OFFSET, &result.bufferOffset);
	glGetTextureLevelParameteriv(_texId, level, GL_TEXTURE_BUFFER_SIZE, &result.bufferSize);
	return result;
}

void Texture::setTexParam(GLenum pname, GLint value) {
	GLint texunit;

	if (!_ready)
		ERRT("No se pueden establecer parámetros de una textura si no está lista");

	texunit = activateScratchTextureUnit();
	glBindTexture(_texture_type, _texId);
	glTexParameteri(_texture_type, pname, value);
	deactivateScratchTextureUnit(texunit);
}


void Texture::setTexParam(GLenum pname, const GLfloat* value) {
	GLint texunit;

	if (!_ready)
		ERRT("No se pueden establecer parámetros de una textura si no está lista");

	texunit = activateScratchTextureUnit();
	glBindTexture(_texture_type, _texId);
	glTexParameterfv(_texture_type, pname, value);
	deactivateScratchTextureUnit(texunit);
}


void Texture::generateMipmap() {
	GLint texunit;
	if (!_ready)
		ERRT("No se pueden generar el mipmap si la textura no está lista");

	glGetIntegerv(GL_ACTIVE_TEXTURE, &texunit);
	if (texunit != (GL_TEXTURE0 + App::getScratchUnitTextureNumber()))
		glActiveTexture(GL_TEXTURE0 + App::getScratchUnitTextureNumber());
	glBindTexture(_texture_type, _texId);
	glGenerateMipmap(_texture_type);
	if (texunit != (GL_TEXTURE0 + App::getScratchUnitTextureNumber()))
		glActiveTexture(GL_TEXTURE0 + App::getScratchUnitTextureNumber());
}
