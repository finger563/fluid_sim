
#include <cstdlib>

#include <cstring>
#include <string>

#include <vector>
#include <string>

#include <math.h>

#include <chrono>
#include <thread>


#include <glad/glad.h>

#include <GLFW/glfw3.h>

GLFWwindow* window;

#define  LOGI(...)  printf(__VA_ARGS__)
#define  LOGE(...)  printf(__VA_ARGS__)


inline void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("OpenGL error %08x, at %s:%i - for %s.\n", err, fname, line, stmt);
	}
}

#ifdef NDEBUG
// helper macro that checks for GL errors.
#define GL_C(stmt) do {					\
	stmt;						\
    } while (0)
#else
// helper macro that checks for GL errors.
#define GL_C(stmt) do {					\
	stmt;						\
	CheckOpenGLError(#stmt, __FILE__, __LINE__);	\
    } while (0)
#endif

inline char* GetShaderLogInfo(GLuint shader) {
	GLint len;
	GLsizei actualLen;
	GL_C(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len));
	char* infoLog = new char[len];
	GL_C(glGetShaderInfoLog(shader, len, &actualLen, infoLog));
	return infoLog;
}

inline GLuint CreateShaderFromString(const std::string& shaderSource, const GLenum shaderType) {
	GLuint shader;

	GL_C(shader = glCreateShader(shaderType));
	const char *c_str = shaderSource.c_str();
	GL_C(glShaderSource(shader, 1, &c_str, NULL));
	GL_C(glCompileShader(shader));

	GLint compileStatus;
	GL_C(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));
	if (compileStatus != GL_TRUE) {
		LOGI("Could not compile shader\n\n%s \n\n%s\n", shaderSource.c_str(),
			GetShaderLogInfo(shader));
		exit(1);
	}

	return shader;
}


inline GLuint LoadNormalShader(const std::string& vsSource, const std::string& fsShader) {

	std::string prefix = "";
	prefix = "#version 330\n";

	GLuint vs = CreateShaderFromString(prefix + vsSource, GL_VERTEX_SHADER);
	GLuint fs = CreateShaderFromString(prefix + fsShader, GL_FRAGMENT_SHADER);

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vs);
	glAttachShader(shader, fs);
	glLinkProgram(shader);

	GLint Result;
	glGetProgramiv(shader, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		LOGI("Could not link shader \n\n%s\n", GetShaderLogInfo(shader));
		exit(1);
	}

	glDetachShader(shader, vs);
	glDetachShader(shader, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return shader;
}

class vec2 {
public:
	float x, y;

	vec2(float x, float y) { this->x = x; this->y = y; }

	vec2() { this->x = this->y = 0; }
};

//
//
// Begin vec3.
//
//

class vec3 {
public:
	float x, y, z;

	vec3(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }

	vec3() { this->x = this->y = this->z = 0; }

	vec3& operator+=(const vec3& b) { (*this) = (*this) + b; return (*this); }

	friend vec3 operator-(const vec3& a, const vec3& b) { return vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
	friend vec3 operator+(const vec3& a, const vec3& b) { return vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
	friend vec3 operator*(const float s, const vec3& a) { return vec3(s * a.x, s * a.y, s * a.z); }
	friend vec3 operator*(const vec3& a, const float s) { return s * a; }

	static float length(const vec3& a) { return sqrt(vec3::dot(a, a)); }

	// dot product.
	static float dot(const vec3& a, const vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

	static float distance(const vec3& a, const vec3& b) { return length(a - b); }
	static vec3 normalize(const vec3& a) { return (1.0f / vec3::length(a)) * a; }

	static vec3 mix(const vec3& x, const vec3& y, const float a) { return x * (1.0f - a) + y * a; }


	// cross product.
	static vec3 cross(const vec3& a, const vec3& b) { return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

	//
	// Rotate the vector 'v' around the 'axis' for 'theta' degrees.
	//
	static vec3 rotate(const vec3& v, const float theta, const vec3& axis) {
		vec3 k = vec3::normalize(axis); // normalize for good measure.
		return v * cos(theta) + vec3::cross(k, v)* sin(theta) + (k * vec3::dot(k, v)) * (1.0f - cos(theta));
	}

};

class vec4 {
public:
	float x, y, z, w;

	vec4(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }

	vec4() { this->x = this->y = this->z = 0; this->w = 0; }

	vec4& operator+=(const vec4& b) { (*this) = (*this) + b; return (*this); }

	friend vec4 operator-(const vec4& a, const vec4& b) { return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
	friend vec4 operator+(const vec4& a, const vec4& b) { return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
	friend vec4 operator*(const float s, const vec4& a) { return vec4(s * a.x, s * a.y, s * a.z, s * a.w); }
	friend vec4 operator*(const vec4& a, const float s) { return s * a; }
	/*
	static float length(const vec3& a) { return sqrt(vec3::dot(a, a)); }

	// dot product.
	static float dot(const vec3& a, const vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

	static float distance(const vec3& a, const vec3& b) { return length(a - b); }
	static vec3 normalize(const vec3& a) { return (1.0f / vec3::length(a)) * a; }
	*/
	//
	// Rotate the vector 'v' around the 'axis' for 'theta' degrees.
	// This is basically Rodrigues' rotation formula.
	//
};


const int WINDOW_WIDTH = 256;
const int WINDOW_HEIGHT = 256;

GLuint vao;

int FRAME_RATE = 30;
int fbWidth, fbHeight;

int frameW, frameH;

struct FullscreenVertex {
	float x, y; // position
};

GLuint fullscreenVertexVbo;

GLuint blitShader;
GLuint bsTexLocation;

GLuint visShader;
GLuint vsTexLocation;

GLuint fbo0;
GLuint fbo1;

GLuint uTex;

void error_callback(int error, const char* description)
{
	puts(description);
}

#ifdef WIN32
void InitGlfw() {
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Deferred Shading Demo", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	// load GLAD.
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// Bind and create VAO, otherwise, we can't do anything in OpenGL.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
}
#endif

void RenderFullscreen() {
	GL_C(glEnableVertexAttribArray((GLuint)0));
	GL_C(glBindBuffer(GL_ARRAY_BUFFER, fullscreenVertexVbo));
	GL_C(glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, sizeof(FullscreenVertex), (void*)0));

	GL_C(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void renderFrame() {

	// StartTimingEvent();

	static bool firstFrame = true;
	// setup matrices.

	//profiler->Begin(GpuTimeStamp::RENDER_EVERYTHING);

	// setup GL state.
	GL_C(glEnable(GL_DEPTH_TEST));
	GL_C(glDepthMask(true));
	GL_C(glDisable(GL_BLEND));
	GL_C(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	GL_C(glEnable(GL_CULL_FACE));
	GL_C(glFrontFace(GL_CCW));
	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_C(glUseProgram(0));
	GL_C(glBindTexture(GL_TEXTURE_2D, 0));
	GL_C(glDepthFunc(GL_LESS));


	GL_C(glViewport(0, 0, frameW, frameH));
	

	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, fbo0));
	GL_C(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uTex, 0));
	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, 0));


	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, fbo0));
	{

		GL_C(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		GL_C(glUseProgram(blitShader));

		RenderFullscreen();
	}
	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
	{
		GL_C(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_C(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		GL_C(glUseProgram(visShader));

		GL_C(glUniform1i(vsTexLocation, 0));
		GL_C(glActiveTexture(GL_TEXTURE0 + 0));
		GL_C(glBindTexture(GL_TEXTURE_2D, uTex));

		RenderFullscreen();
	}
	
}


#ifdef WIN32
void HandleInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}
#endif

void checkFbo() {
	// make sure nothing went wrong:
	GLenum status;
	GL_C(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOGE("Framebuffer not complete. Status: 0x%08x\n", status);
	}
}

GLuint createFloatTexture() {
	GLuint tex;

	GL_C(glGenTextures(1, &tex));
	GL_C(glBindTexture(GL_TEXTURE_2D, tex));
	GL_C(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameW, frameH, 0, GL_RGBA, GL_FLOAT, nullptr));
	GL_C(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_C(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GL_C(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_C(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GL_C(glBindTexture(GL_TEXTURE_2D, 0));

	return tex;
}

void setupGraphics(int w, int h) {

#ifdef WIN32
	InitGlfw();
#else
	fbWidth = w;
	fbHeight = h;
#endif

	frameW = fbWidth;
	frameH = fbHeight;

	// frameFbo
	{
		{
			uTex = createFloatTexture();
		}

		{
			GL_C(glGenFramebuffers(1, &fbo0));
			GL_C(glGenFramebuffers(1, &fbo1));
		}
	}

	std::string fullscreenVs(R"(
       layout(location = 0) in vec3 vsPos;

        out vec2 fsUv;

        void main() {
          fsUv = remap(vsPos.xy);
          gl_Position =  vec4(2.0 * remap(vsPos.xy) - vec2(1.0), 0.0, 1.0);
        }
		)");

	vec2 delta(1.0f / frameW, 1.0f / frameH);

	std::string fragDefines = "";
	fragDefines += std::string("#define delta vec2(") + std::to_string(delta.x) + std::string(",") + std::to_string(delta.y) + ")\n";

	std::string vertDefines = "";
	vertDefines += fragDefines;
	vertDefines += std::string("vec2 remap(vec2 p) { return delta + p * (1.0 - 2.0 * delta); }\n");

	blitShader = LoadNormalShader(
		vertDefines +
		fullscreenVs,
		fragDefines +
		std::string(R"(

        in vec2 fsUv;

      uniform sampler2D uTex;
        //uniform sampler2D uDownsampleTex;

        //uniform vec2 uScreenSpaceLightPos;

#define NUM_SAMPLES 100
        out vec4 FragColor;

		void main()
		{
          FragColor = vec4(fsUv, 0.0,  1.0);
        //  FragColor = vec4(1.0, 0.0, 0.0,  1.0);
		}
		)")
	);
	bsTexLocation = glGetUniformLocation(blitShader, "uColorTex");
	
	visShader = LoadNormalShader(
		
		std::string("vec2 remap(vec2 p) { return p; }\n") +

		fullscreenVs,

		std::string(R"(

        in vec2 fsUv;

        uniform sampler2D uTex;

        out vec4 FragColor;

		void main()
		{
          FragColor = vec4(texture2D(uTex, fsUv).rgb, 1.0);
		}
		)")
	);
	vsTexLocation = glGetUniformLocation(visShader, "uTex");
	
	{
		std::vector<FullscreenVertex> vertices;

		vertices.push_back(FullscreenVertex{ +0.0f, +0.0f });
		vertices.push_back(FullscreenVertex{ +1.0f, +0.0f });
		vertices.push_back(FullscreenVertex{ +0.0f, +1.0f });

		vertices.push_back(FullscreenVertex{ +1.0f, +0.0f });
		vertices.push_back(FullscreenVertex{ +1.0f, +1.0f });
		vertices.push_back(FullscreenVertex{ +0.0f, +1.0f });

		// upload geometry to GPU.
		GL_C(glGenBuffers(1, &fullscreenVertexVbo));
		GL_C(glBindBuffer(GL_ARRAY_BUFFER, fullscreenVertexVbo));
		GL_C(glBufferData(GL_ARRAY_BUFFER, sizeof(FullscreenVertex)*vertices.size(), (float*)vertices.data(), GL_STATIC_DRAW));
	}

	
	LOGI("start loading extension!\n");

	LOGI("Init opengl\n");
}
