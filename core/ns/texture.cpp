#include "texture.h"
#include "../ew/external/stb_image.h"
#include "../ew/external/glad.h"

namespace ns {
	unsigned int loadTexture(const char* filePath, int wrapMode, int filterMode) {

		stbi_set_flip_vertically_on_load(true);

		int width, height, numComponents;
		unsigned char* data = stbi_load(filePath, &width, &height, &numComponents, 0);
		if (data == NULL) {
			printf("Failed to load image %s", filePath);
			stbi_image_free(data);
			return 0;
		}

		//Create and bind new texture name
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		//Automatically set format and internal format based on num of components
		GLenum format;
		GLint internalFormat;

		switch (numComponents)
		{
		case 1:
			format = GL_RED;
			internalFormat = GL_RED;
			break;
		case 2:
			format = GL_RG;
			internalFormat = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			internalFormat = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = GL_RGBA;
			break;
		}

		//Reserve memory and set texture data
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		//Setting wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

		//Setting filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

		//Generating mipmap levels
		glGenerateMipmap(GL_TEXTURE_2D);

		//Return handle
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
		return texture;
	}
}