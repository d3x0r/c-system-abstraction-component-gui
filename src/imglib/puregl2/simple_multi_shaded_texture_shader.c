#include <stdhdrs.h>

#ifdef USE_GLES2
//#include <GLES/gl.h>
#include <GLES2/gl2.h>
#else
#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glu.h>
#endif

#include "local.h"

#include "shaders.h"


//const char *gles_
static const CTEXTSTR gles_simple_v_multi_shader =
	WIDE( "uniform mat4 modelView;\n" )
	WIDE( "uniform mat4 worldView;\n" )
	WIDE( "uniform mat4 Projection;\n" )
	WIDE( "attribute vec2 in_texCoord;\n" )
    WIDE( "attribute vec4 vPosition;" )
	WIDE( " varying vec2 out_texCoord;\n" )
	//WIDE( "in  vec4 in_Color;\n")
	//WIDE( "out vec4 ex_Color;\n")
    WIDE( "void main(void) {" )
    WIDE( "  gl_Position = Projection * worldView * vPosition;" )
	WIDE( "out_texCoord = in_texCoord;\n" )
	//WIDE( "  ex_Color = in_Color;" )
    WIDE( "}"); 


static const CTEXTSTR gles_simple_p_multi_shader =
	    WIDE( " varying vec2 out_texCoord;\n" )
       WIDE( "uniform sampler2D tex;\n" )
       WIDE( "uniform vec4 multishade_r;\n" )
       WIDE( "uniform vec4 multishade_g;\n" )
       WIDE( "uniform vec4 multishade_b;\n" )
       WIDE( "\n" )
       WIDE( "void main(void)\n" )
       WIDE( "{\n" )
       WIDE( "    vec4 color = texture2D(tex, out_texCoord);\n" )
       WIDE( "	gl_FragColor = vec4( (color.b * multishade_b.r) + (color.g * multishade_g.r) + (color.r * multishade_r.r),\n" )
       WIDE( "		(color.b * multishade_b.g) + (color.g * multishade_g.g) + (color.r * multishade_r.g),\n" )
       WIDE( "		(color.b * multishade_b.b) + (color.g * multishade_g.b) + (color.r * multishade_r.b),\n" )
       WIDE( "		  color.r!=0.0?( color.a * multishade_r.a) :0.0\n" )
       WIDE( "                + color.g!=0.0?( color.a * multishade_g.a) :0.0\n" )
       WIDE( "                + color.b!=0.0?( color.a * multishade_b.a) :0.0\n" )
       WIDE( "                )\n" )
       WIDE( "		;\n" )
				WIDE( "}\n" );


struct private_mst_shader_data
{
	int texture_attrib;
	int texture;
	int r_color_attrib;
	int g_color_attrib;
	int b_color_attrib;
};


static void CPROC SimpleMultiShadedTextureEnable( PImageShaderTracker tracker, va_list args )
{
	float *verts = va_arg( args, float *);
	int texture = va_arg( args, int );
	float *texture_verts = va_arg( args, float *);
	float *r_color = va_arg( args, float *);
	float *g_color = va_arg( args, float *);
	float *b_color = va_arg( args, float *);

	struct private_mst_shader_data *data = (struct private_mst_shader_data *)tracker->psv_userdata;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer( 0, 3, GL_FLOAT, FALSE, 0, verts );  
	CheckErr();
	
	glEnableVertexAttribArray(data->texture_attrib);
	glVertexAttribPointer( data->texture_attrib, 2, GL_FLOAT, FALSE, 0, texture_verts );  
	CheckErr();

	glUniform1i(data->texture, 0); //Texture unit 0 is for base images.
 
	//When rendering an objectwith this program.
	glActiveTexture(GL_TEXTURE0 + 0);
	CheckErr();
	glBindTexture(GL_TEXTURE_2D, texture);
	CheckErr();
	//glBindSampler(0, linearFiltering);
	CheckErr();

	glUniform4fv( data->r_color_attrib, 1, r_color );
	CheckErr();
	glUniform4fv( data->g_color_attrib, 1, g_color );
	CheckErr();
	glUniform4fv( data->b_color_attrib, 1, b_color );
	CheckErr();


}


void InitSimpleMultiShadedTextureShader( PImageShaderTracker tracker )
{
	GLint result;
	const char *v_codeblocks[2];
	const char *p_codeblocks[2];
	struct private_mst_shader_data *data = New( struct private_mst_shader_data );
	struct image_shader_attribute_order attribs[] = { { 0, "vPosition" }, { 1, "in_TexCoord" } };

	tracker->psv_userdata = (PTRSZVAL)data;
	tracker->Enable = SimpleMultiShadedTextureEnable;

	if( result = glGetError() )
	{
		lprintf( "unhandled error before shader" );
	}

	v_codeblocks[0] = gles_simple_v_multi_shader;
	v_codeblocks[1] = NULL;
	p_codeblocks[0] = gles_simple_p_multi_shader;
	p_codeblocks[1] = NULL;
	if( CompileShaderEx( tracker, v_codeblocks, 1, p_codeblocks, 1, attribs, 2 ) )
	{
		SetupCommon( tracker, "vPosition", "in_Color" );
		
		data->r_color_attrib = glGetUniformLocation(tracker->glProgramId, "multishade_r" );
		CheckErr();
		data->g_color_attrib = glGetUniformLocation(tracker->glProgramId, "multishade_g" );
		CheckErr();
		data->b_color_attrib = glGetUniformLocation(tracker->glProgramId, "multishade_b" );
		CheckErr();
		data->texture = glGetUniformLocation(tracker->glProgramId, "tex");
		CheckErr();
		data->texture_attrib =  glGetAttribLocation(tracker->glProgramId, "in_texCoord" );
		CheckErr();
	}
}
