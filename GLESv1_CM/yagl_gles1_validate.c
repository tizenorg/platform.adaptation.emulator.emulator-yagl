#include "GLES/gl.h"
#include "yagl_gles1_validate.h"

int yagl_gles1_get_texenv_param_count(GLenum type, int *count)
{
    switch (type) {
    case GL_TEXTURE_ENV_COLOR:
        *count = 4;
        break;
    case GL_TEXTURE_ENV_MODE:
    case GL_COMBINE_RGB:
    case GL_COMBINE_ALPHA:
    case GL_SRC0_RGB:
    case GL_SRC1_RGB:
    case GL_SRC2_RGB:
    case GL_SRC0_ALPHA:
    case GL_SRC1_ALPHA:
    case GL_SRC2_ALPHA:
    case GL_OPERAND0_RGB:
    case GL_OPERAND1_RGB:
    case GL_OPERAND2_RGB:
    case GL_OPERAND0_ALPHA:
    case GL_OPERAND1_ALPHA:
    case GL_OPERAND2_ALPHA:
    case GL_RGB_SCALE:
    case GL_ALPHA_SCALE:
    case GL_COORD_REPLACE_OES:
        *count = 1;
        break;
    default:
        return 0;
    }
    return 1;
}

int yagl_gles1_get_point_param_count(GLenum pname, int *count)
{
    switch (pname) {
    case GL_POINT_DISTANCE_ATTENUATION:
        *count = 3;
        break;
    case GL_POINT_SIZE_MIN:
    case GL_POINT_SIZE_MAX:
    case GL_POINT_FADE_THRESHOLD_SIZE:
        *count = 1;
        break;
    default:
        return 0;
    }
    return 1;
}

int yagl_gles1_get_fog_param_count(GLenum pname, int *count)
{
    switch (pname) {
    case GL_FOG_COLOR:
        *count = 4;
        break;
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        *count = 1;
        break;
    default:
        return 0;
    }
    return 1;
}

int yagl_gles1_get_light_param_count(GLenum pname, int *count)
{
    switch (pname) {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_POSITION:
        *count = 4;
        break;
    case GL_SPOT_DIRECTION:
        *count = 3;
        break;
    case GL_SPOT_EXPONENT:
    case GL_SPOT_CUTOFF:
    case GL_CONSTANT_ATTENUATION:
    case GL_LINEAR_ATTENUATION:
    case GL_QUADRATIC_ATTENUATION:
        *count = 1;
        break;
    default:
        return 0;
    }
    return 1;
}

int yagl_gles1_get_light_model_param_count(GLenum pname, int *count)
{
    switch (pname) {
    case GL_LIGHT_MODEL_AMBIENT:
        *count = 4;
        break;
    case GL_LIGHT_MODEL_TWO_SIDE:
        *count = 1;
        break;
    default:
        return 0;
    }
    return 1;
}

int yagl_gles1_get_material_param_count(GLenum pname, int *count)
{
    switch (pname) {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_EMISSION:
    case GL_AMBIENT_AND_DIFFUSE:
        *count = 4;
        break;
    case GL_SHININESS:
        *count = 1;
        break;
    default:
        return 0;
    }
    return 1;
}
