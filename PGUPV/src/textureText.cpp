
#include <assert.h>
#include <cstring>

#include "glStateCache.h"
#include "textureText.h"
#include "log.h"
#include "app.h"


#ifdef WIN32
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

using PGUPV::TextureTextBuilder;
using PGUPV::TextureText;
using PGUPV::Font;


TextureTextBuilder::TextureTextBuilder() {
	theFont = Font::getDefaultFont();
	theText = " ";
	theWrapWidth = 0;
	fgcolor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
}

TextureTextBuilder& TextureTextBuilder::font(std::shared_ptr<Font> font) {
	this->theFont = font;
	return *this;
}

TextureTextBuilder& TextureTextBuilder::text(const std::string& text) {
	this->theText = text;
	return *this;
}

TextureTextBuilder& TextureTextBuilder::wrapWidth(uint pixels) {
	this->theWrapWidth = pixels;
	return *this;
}

TextureTextBuilder& TextureTextBuilder::foregroundColor(const glm::vec4& color) {
	this->fgcolor = color;
	return *this;
}

TextureTextBuilder& TextureTextBuilder::backgroundColor(const glm::vec4& color) {
	this->bgcolor = color;
	return *this;
}

std::shared_ptr<TextureText> TextureTextBuilder::build() {

	PGUPV::GLStateCapturer< PGUPV::ActiveTextureUnitState> restoreActiveTextureUnit;
	glActiveTexture(GL_TEXTURE0 + PGUPV::App::getScratchUnitTextureNumber());
 	auto result = std::shared_ptr<TextureText>(new TextureText(theText, theFont, theWrapWidth, fgcolor, bgcolor));
	result->setName(theText);
	return result;
}



TextureText::TextureText(const std::string& t, std::shared_ptr<Font> font, uint wrapWidth, const glm::vec4& fgcolor, const glm::vec4& bgcolor) :
	Texture2D(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE) {

	if (t.length() == 0) {
		ERRT("No se puede construir la textura de la cadena vacía");
	}

	SDL_Color sdlFGColor{ Uint8(fgcolor.r * 255), Uint8(fgcolor.g * 255), Uint8(fgcolor.b * 255), Uint8(fgcolor.a * 255) };
	SDL_Color sdlBGColor{ Uint8(bgcolor.r * 255), Uint8(bgcolor.g * 255), Uint8(bgcolor.b * 255), Uint8(bgcolor.a * 255) };
	SDL_Surface* textSurface;

	if (wrapWidth == 0) {
		textSurface = TTF_RenderUTF8_Shaded(font->getTTFFont(), t.c_str(), sdlFGColor, sdlBGColor);
	}
	else {
		textSurface = TTF_RenderUTF8_Blended_Wrapped(font->getTTFFont(), t.c_str(), sdlFGColor, wrapWidth);
	}


	if (textSurface == NULL)
		ERRT(std::string("No se ha podido generar el texto. Error SDL_ttf: ") + TTF_GetError());

#ifdef _DEBUG
	assert(textSurface->pitch % 4 == 0);
#endif

	uint32_t width, height;
	std::vector<uchar> temp;
	if (textSurface->format->palette) {
		width = textSurface->w;
		height = textSurface->h;
		temp.resize(width * height * 4);
		uchar* wtemp = &temp[0], * rtemp;
		for (uint32_t i = 0; i < height; i++) {
			rtemp = ((uchar*)textSurface->pixels) + (height - i - 1) * textSurface->pitch;
			for (uint32_t j = 0; j < width; j++) {
				memcpy(wtemp, &textSurface->format->palette->colors[*rtemp].r, 4);
				wtemp += 4;
				rtemp++;
			}
		}
	}
	else {
		width = textSurface->pitch / 4;
		height = textSurface->h;
		temp.resize( width * height * 4 );
		for (uint32_t i = 0; i < height; i++) {
			memcpy(&temp[0] + textSurface->pitch * i,
				((uchar*)textSurface->pixels) + (height - 1 - i) * textSurface->pitch,
				textSurface->pitch);
		}
	}

	FRAME("Tamaño de la textura generada: " + std::to_string(width) + ", " + std::to_string(height));

	/*  if (textSurface->format->BitsPerPixel == 8)
		loadImageFromMemory(temp, textSurface->w, textSurface->h, GL_RED, GL_UNSIGNED_BYTE, GL_RGBA);
	  else if (textSurface->format->BitsPerPixel == 24)
		loadImageFromMemory(temp, textSurface->w, textSurface->h, GL_RGB, GL_UNSIGNED_BYTE, GL_RGBA);
	  else if (textSurface->format->BitsPerPixel == 32)*/

	loadImageFromMemory(&temp[0], width, height, GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA);
	/*else
	  ERRT("Formato de imagen no soportado");
	  */

	SDL_FreeSurface(textSurface);
}
