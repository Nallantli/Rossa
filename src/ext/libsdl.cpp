#include "libsdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>

#ifndef _STATIC_
#ifdef __unix__
COMPILER_COMMANDS(libsdl, "-lSDL2 -lSDL2_image -lSDL2_ttf")
#else
COMPILER_COMMANDS(libsdl, "-lmingw32 -lgdi32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf")
#endif
#endif

namespace libsdl
{
	typedef unsigned char color_t;

	static std::map<Uint32, sym_t> registered = {};

	struct Image
	{
	private:
		SDL_Surface *loaded = NULL;
		SDL_Texture *image = NULL;

	public:
		Image(const std::string &path, const token_t *token, trace_t &stack_trace, const color_t &r, const color_t &g, const color_t &b)
		{
			loaded = IMG_Load(path.c_str());
			if (loaded == NULL)
				throw RTError(format::format("Texture file `{0}` loading error: {1}", { path, IMG_GetError() }), *token, stack_trace);
			SDL_SetColorKey(loaded, SDL_TRUE, SDL_MapRGB(loaded->format, r, g, b));
		}

		Image(const std::string &path, const token_t *token, trace_t &stack_trace)
		{
			loaded = IMG_Load(path.c_str());
			if (loaded == NULL)
				throw RTError(format::format("Texture file `{0}` loading error: {1}", { path, IMG_GetError() }), *token, stack_trace);
		}

		SDL_Texture *getImage(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace)
		{
			if (image == NULL) {
				image = SDL_CreateTextureFromSurface(renderer, loaded);
				if (image == NULL)
					throw RTError(format::format("Cannot create renderable image: {0}", { SDL_GetError() }), *token, stack_trace);
				SDL_FreeSurface(loaded);
				loaded = NULL;
			}
			return image;
		}

		~Image()
		{
			if (loaded != NULL)
				SDL_FreeSurface(loaded);
			if (image != NULL)
				SDL_DestroyTexture(image);
		}
	};

	struct Font
	{
		TTF_Font *font;

		Font(const std::string &path, const int &size, const token_t *token, trace_t &stack_trace)
		{
			font = TTF_OpenFont(path.c_str(), size);
			if (font == NULL)
				throw RTError(format::format("Failure to initialize font: {0}", { TTF_GetError() }), *token, stack_trace);
		}

		~Font()
		{
			TTF_CloseFont(font);
		}
	};

	struct Shape
	{
		color_t r, g, b, a;
		const hash_ull id;
		static hash_ull id_count;

		Shape(const color_t &r, const color_t &b, const color_t &g, const color_t &a)
			: r{ r }
			, g{ g }
			, b{ b }
			, a{ a }
			, id{ id_count++ }
		{}

		virtual void draw(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace, const int &x, const int &y) = 0;

		void setColor(const color_t &r, const color_t &g, const color_t &b, const color_t &a)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		bool operator==(const Shape &s) const
		{
			return id == s.id;
		}
	};

	hash_ull Shape::id_count = 0;

	struct Sizable : public Shape
	{
		int width;
		int height;

		Sizable(const int &width, const int &height, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a)
			, width{ width }
			, height{ height }
		{}

		void setSize(const int &width, const int &height)
		{
			this->width = width;
			this->height = height;
		}

		void setWidth(const int &width)
		{
			this->width = width;
		}

		void setHeight(const int &height)
		{
			this->height = height;
		}
	};

	struct Rotatable : public Sizable
	{
		double angle = 0;
		SDL_Point *center = NULL;
		SDL_Rect *clip = NULL;

		Rotatable(const int &width, const int &height, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Sizable(width, height, r, g, b, a)
		{}

		void setAngle(const double &angle)
		{
			this->angle = angle;
		}

		void setCenter(const int &x, const int &y)
		{
			if (center != NULL)
				SDL_free(center);
			center = new SDL_Point({ x, y });
		}

		void deCenter()
		{
			if (center != NULL)
				SDL_free(center);
			center = NULL;
		}

		void setClip(const int &cx, const int &cy, const int &cwidth, const int &cheight)
		{
			if (clip != NULL)
				SDL_free(clip);
			clip = new SDL_Rect({ cx, cy, cwidth, cheight });
		}

		void deClip()
		{
			if (clip != NULL)
				SDL_free(clip);
			clip = NULL;
		}

		~Rotatable()
		{
			if (center != NULL)
				SDL_free(center);
			if (clip != NULL)
				SDL_free(clip);
		}
	};

	struct Rectangle : public Sizable
	{
		Rectangle(const int &width, const int &height, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Sizable(width, height, r, g, b, a)
		{}

		void draw(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace, const int &x, const int &y) override
		{
			SDL_Rect temp = { x, y, width, height };
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RTError(format::format("Error setting shape color: {0}", { SDL_GetError() }), *token, stack_trace);
			if (SDL_RenderFillRect(renderer, &temp) < 0)
				throw RTError(format::format("Error drawing shape: {0}", { SDL_GetError() }), *token, stack_trace);
		}
	};

	struct Line : public Shape
	{
		int x1;
		int y1;
		Line(const int &x1, const int &y1, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a)
			, x1{ x1 }
			, y1{ y1 }
		{}

		void draw(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace, const int &x, const int &y) override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RTError(format::format("Error setting shape color: {0}", { SDL_GetError() }), *token, stack_trace);
			if (SDL_RenderDrawLine(renderer, x, y, x1, y1) < 0)
				throw RTError(format::format("Error drawing shape: {0}", { SDL_GetError() }), *token, stack_trace);
		}
	};

	struct Point : public Shape
	{
		Point(const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a)
		{}

		void draw(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace, const int &x, const int &y) override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RTError(format::format("Error setting shape color: {0}", { SDL_GetError() }), *token, stack_trace);
			if (SDL_RenderDrawPoint(renderer, x, y) < 0)
				throw RTError(format::format("Error drawing shape: {0}", { SDL_GetError() }), *token, stack_trace);
		}
	};

	struct Texture : public Rotatable
	{
		sym_t image;

		Texture(const sym_t &image, const int &width, const int &height, const color_t &r, const color_t &g, const color_t &b)
			: Rotatable(width, height, r, g, b, 0)
			, image{ image }
		{}

		void setImage(const sym_t &image)
		{
			this->image = image;
		}

		void draw(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace, const int &x, const int &y) override
		{
			auto img = COERCE_PTR(image.getPointer(token, stack_trace), Image)->getImage(renderer, token, stack_trace);
			SDL_SetTextureColorMod(img, r, g, b);
			SDL_Rect temp = { x, y, width, height };
			SDL_RenderCopyEx(renderer, img, clip, &temp, angle, center, SDL_FLIP_NONE);
		}
	};

	struct Renderer
	{
		Uint32 windowID;
		SDL_Renderer *renderer = NULL;
		std::vector<std::pair<sym_t, std::pair<int, int>>> shapes;

		Renderer(SDL_Window *window, const token_t *token, trace_t &stack_trace)
		{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL)
				throw RTError(format::format("Failure to initialize renderer: {0}", { SDL_GetError() }), *token, stack_trace);
		}

		void addShape(const sym_t &shape, const int &x, const int &y)
		{
			shapes.push_back({ shape, {x, y} });
		}

		void clearAll()
		{
			shapes.clear();
		}

		void draw(const token_t *token, trace_t &stack_trace)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			for (auto &s : shapes)
				COERCE_PTR(s.first.getPointer(token, stack_trace), Shape)->draw(renderer, token, stack_trace, s.second.first, s.second.second);
			SDL_RenderPresent(renderer);
		}

		~Renderer()
		{
			SDL_DestroyRenderer(renderer);
		}
	};

	struct Text : Shape
	{
	private:
		SDL_Surface *loaded = NULL;
		SDL_Texture *image = NULL;
		int width;
		int height;
		std::string text;
		sym_t font;

	public:
		Text(const sym_t &font, const std::string &s, const token_t *token, trace_t &stack_trace, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a)
		{
			this->font = font;
			setText(s, token, stack_trace);
		}

		SDL_Texture *renderFont(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace)
		{
			if (image == NULL) {
				image = SDL_CreateTextureFromSurface(renderer, loaded);
				if (image == NULL)
					throw RTError(format::format("Cannot create renderable image: {0}", { SDL_GetError() }), *token, stack_trace);
				SDL_QueryTexture(image, NULL, NULL, &width, &height);
				SDL_FreeSurface(loaded);
				loaded = NULL;
			}
			return image;
		}

		void setText(const std::string &s, const token_t *token, trace_t &stack_trace)
		{
			if (s == text && loaded != NULL)
				return;
			freeData();
			if (s == "")
				return;
			auto fdata = (COERCE_PTR(font.getPointer(token, stack_trace), libsdl::Font))->font;
			loaded = TTF_RenderText_Solid(fdata, s.c_str(), { r, g, b, a });
			if (loaded == NULL)
				throw RTError(format::format("Font rendering error: {0}", { TTF_GetError() }), *token, stack_trace);
			text = s;
		}

		void draw(SDL_Renderer *renderer, const token_t *token, trace_t &stack_trace, const int &x, const int &y) override
		{
			if (text == "")
				return;
			SDL_Rect temp = { x, y, width, height };
			SDL_RenderCopy(renderer, renderFont(renderer, token, stack_trace), NULL, &temp);
		}

		void freeData()
		{
			if (loaded != NULL)
				SDL_FreeSurface(loaded);
			loaded = NULL;
			if (image != NULL)
				SDL_DestroyTexture(image);
			image = NULL;
		}

		~Text()
		{
			freeData();
		}
	};

	struct Window
	{
		Uint32 windowID;
		SDL_Window *window = NULL;

		Window(const std::string &title, const int &width, const int &height, const token_t *token, trace_t &stack_trace)
		{
			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
			if (window == NULL)
				throw RTError(format::format("Failure to initialize window: {0}", { SDL_GetError() }), *token, stack_trace);

			this->windowID = SDL_GetWindowID(window);
		}

		std::shared_ptr<Renderer> getRenderer(const token_t *token, trace_t &stack_trace)
		{
			auto g = std::make_shared<Renderer>(window, token, stack_trace);
			return g;
		}

		~Window()
		{
			libsdl::registered.erase(windowID);
			SDL_DestroyWindow(window);
		}
	};
}

ROSSA_EXT_SIG(_sdl_init, args, token, hash, stack_trace)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		throw RTError(format::format("Failure to initialize SDL: {0}", { SDL_GetError() }), *token, stack_trace);

	int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
	if (!(IMG_Init(imgFlags) & imgFlags))
		throw RTError(format::format("Failure to initialize SDL_image: {0}", { IMG_GetError() }), *token, stack_trace);

	if (TTF_Init() < 0)
		throw RTError(format::format("Failure to initialize SDL_ttf: {0}", { TTF_GetError() }), *token, stack_trace);

	return sym_t();
}

ROSSA_EXT_SIG(_sdl_quit, args, token, hash, stack_trace)
{
	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	return sym_t();
}

ROSSA_EXT_SIG(_window_init, args, token, hash, stack_trace)
{
	auto w = std::make_shared<libsdl::Window>(args[0].getString(token, stack_trace), args[1].getNumber(token, stack_trace).getLong(), args[2].getNumber(token, stack_trace).getLong(), token, stack_trace);
	sym_vec_t v = { sym_t::Pointer(w), sym_t::Number(number_t::Long(w->windowID)) };
	return sym_t::Array(v);
}

ROSSA_EXT_SIG(_event_poll, args, token, hash, stack_trace)
{
	SDL_Event e;
	sym_map_t data;
	if (SDL_PollEvent(&e)) {
		data["type"] = sym_t::Number(number_t::Long(e.type));
		switch (e.type) {
			case SDL_WINDOWEVENT:
				data["timestamp"] = sym_t::Number(number_t::Long(e.window.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.window.windowID));
				data["window"] = libsdl::registered[e.window.windowID];
				data["event"] = sym_t::Number(number_t::Long(e.window.event));
				data["data1"] = sym_t::Number(number_t::Long(e.window.data1));
				data["data2"] = sym_t::Number(number_t::Long(e.window.data2));
				return sym_t::Dictionary(data);
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				data["timestamp"] = sym_t::Number(number_t::Long(e.key.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.key.windowID));
				data["window"] = libsdl::registered[e.key.windowID];
				data["state"] = sym_t::Number(number_t::Long(e.key.state));
				data["repeat"] = sym_t::Number(number_t::Long(e.key.repeat));
				sym_map_t keysym;
				keysym["scancode"] = sym_t::Number(number_t::Long(e.key.keysym.scancode));
				keysym["sym"] = sym_t::Number(number_t::Long(e.key.keysym.sym));
				keysym["mod"] = sym_t::Number(number_t::Long(e.key.keysym.mod));
				data["keysym"] = sym_t::Dictionary(keysym);
				return sym_t::Dictionary(data);
			}
			case SDL_TEXTEDITING:
				data["timestamp"] = sym_t::Number(number_t::Long(e.edit.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.edit.windowID));
				data["window"] = libsdl::registered[e.edit.windowID];
				data["text"] = sym_t::String(std::string(e.edit.text));
				data["start"] = sym_t::Number(number_t::Long(e.edit.start));
				data["length"] = sym_t::Number(number_t::Long(e.edit.length));
				return sym_t::Dictionary(data);
			case SDL_TEXTINPUT:
				data["timestamp"] = sym_t::Number(number_t::Long(e.text.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.text.windowID));
				data["window"] = libsdl::registered[e.text.windowID];
				data["text"] = sym_t::String(std::string(e.text.text));
				return sym_t::Dictionary(data);
			case SDL_MOUSEMOTION:
				data["timestamp"] = sym_t::Number(number_t::Long(e.motion.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.motion.windowID));
				data["window"] = libsdl::registered[e.motion.windowID];
				data["which"] = sym_t::Number(number_t::Long(e.motion.which));
				data["state"] = sym_t::Number(number_t::Long(e.motion.state));
				data["x"] = sym_t::Number(number_t::Long(e.motion.x));
				data["y"] = sym_t::Number(number_t::Long(e.motion.y));
				data["xrel"] = sym_t::Number(number_t::Long(e.motion.xrel));
				data["yrel"] = sym_t::Number(number_t::Long(e.motion.yrel));
				return sym_t::Dictionary(data);
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				data["timestamp"] = sym_t::Number(number_t::Long(e.button.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.button.windowID));
				data["window"] = libsdl::registered[e.button.windowID];
				data["which"] = sym_t::Number(number_t::Long(e.button.which));
				data["state"] = sym_t::Number(number_t::Long(e.button.state));
				data["x"] = sym_t::Number(number_t::Long(e.button.x));
				data["y"] = sym_t::Number(number_t::Long(e.button.y));
				data["button"] = sym_t::Number(number_t::Long(e.button.button));
				data["clicks"] = sym_t::Number(number_t::Long(e.button.clicks));
				return sym_t::Dictionary(data);
			case SDL_MOUSEWHEEL:
				data["timestamp"] = sym_t::Number(number_t::Long(e.wheel.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.wheel.windowID));
				data["window"] = libsdl::registered[e.wheel.windowID];
				data["which"] = sym_t::Number(number_t::Long(e.wheel.which));
				data["direction"] = sym_t::Number(number_t::Long(e.wheel.direction));
				data["x"] = sym_t::Number(number_t::Long(e.wheel.x));
				data["y"] = sym_t::Number(number_t::Long(e.wheel.y));
				return sym_t::Dictionary(data);
			case SDL_JOYAXISMOTION:
				data["timestamp"] = sym_t::Number(number_t::Long(e.jaxis.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.jaxis.which));
				data["axis"] = sym_t::Number(number_t::Long(e.jaxis.axis));
				data["value"] = sym_t::Number(number_t::Long(e.jaxis.value));
				return sym_t::Dictionary(data);
			case SDL_JOYBALLMOTION:
				data["timestamp"] = sym_t::Number(number_t::Long(e.jball.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.jball.which));
				data["ball"] = sym_t::Number(number_t::Long(e.jball.ball));
				data["xrel"] = sym_t::Number(number_t::Long(e.jball.xrel));
				data["yrel"] = sym_t::Number(number_t::Long(e.jball.yrel));
				return sym_t::Dictionary(data);
			case SDL_JOYHATMOTION:
				data["timestamp"] = sym_t::Number(number_t::Long(e.jhat.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.jhat.which));
				data["hat"] = sym_t::Number(number_t::Long(e.jhat.hat));
				data["value"] = sym_t::Number(number_t::Long(e.jhat.value));
				return sym_t::Dictionary(data);
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				data["timestamp"] = sym_t::Number(number_t::Long(e.jbutton.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.jbutton.which));
				data["button"] = sym_t::Number(number_t::Long(e.jbutton.button));
				data["state"] = sym_t::Number(number_t::Long(e.jbutton.state));
				return sym_t::Dictionary(data);
			case SDL_JOYDEVICEADDED:
			case SDL_JOYDEVICEREMOVED:
				data["timestamp"] = sym_t::Number(number_t::Long(e.jdevice.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.jdevice.which));
				return sym_t::Dictionary(data);
			case SDL_CONTROLLERAXISMOTION:
				data["timestamp"] = sym_t::Number(number_t::Long(e.caxis.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.caxis.which));
				data["axis"] = sym_t::Number(number_t::Long(e.caxis.axis));
				data["value"] = sym_t::Number(number_t::Long(e.caxis.value));
				return sym_t::Dictionary(data);
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				data["timestamp"] = sym_t::Number(number_t::Long(e.cbutton.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.cbutton.which));
				data["button"] = sym_t::Number(number_t::Long(e.cbutton.button));
				data["state"] = sym_t::Number(number_t::Long(e.cbutton.state));
				return sym_t::Dictionary(data);
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				data["timestamp"] = sym_t::Number(number_t::Long(e.cdevice.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.cdevice.which));
				return sym_t::Dictionary(data);
			case SDL_AUDIODEVICEADDED:
			case SDL_AUDIODEVICEREMOVED:
				data["timestamp"] = sym_t::Number(number_t::Long(e.adevice.timestamp));
				data["which"] = sym_t::Number(number_t::Long(e.adevice.which));
				data["iscapture"] = sym_t::Number(number_t::Long(e.adevice.iscapture));
				return sym_t::Dictionary(data);
			case SDL_QUIT:
				data["timestamp"] = sym_t::Number(number_t::Long(e.quit.timestamp));
				return sym_t::Dictionary(data);
			case SDL_FINGERMOTION:
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
				data["timestamp"] = sym_t::Number(number_t::Long(e.tfinger.timestamp));
				data["touchId"] = sym_t::Number(number_t::Long(e.tfinger.touchId));
				data["fingerId"] = sym_t::Number(number_t::Long(e.tfinger.fingerId));
				data["x"] = sym_t::Number(number_t::Double(e.tfinger.x));
				data["y"] = sym_t::Number(number_t::Double(e.tfinger.y));
				data["dx"] = sym_t::Number(number_t::Double(e.tfinger.dx));
				data["dy"] = sym_t::Number(number_t::Double(e.tfinger.dy));
				data["pressure"] = sym_t::Number(number_t::Double(e.tfinger.pressure));
				return sym_t::Dictionary(data);
			case SDL_MULTIGESTURE:
				data["timestamp"] = sym_t::Number(number_t::Long(e.mgesture.timestamp));
				data["touchId"] = sym_t::Number(number_t::Long(e.mgesture.touchId));
				data["dTheta"] = sym_t::Number(number_t::Double(e.mgesture.dTheta));
				data["dDist"] = sym_t::Number(number_t::Double(e.mgesture.dDist));
				data["x"] = sym_t::Number(number_t::Double(e.mgesture.x));
				data["y"] = sym_t::Number(number_t::Double(e.mgesture.y));
				data["numFingers"] = sym_t::Number(number_t::Long(e.mgesture.numFingers));
				return sym_t::Dictionary(data);
			case SDL_DOLLARGESTURE:
			case SDL_DOLLARRECORD:
				data["timestamp"] = sym_t::Number(number_t::Long(e.dgesture.timestamp));
				data["touchId"] = sym_t::Number(number_t::Long(e.dgesture.touchId));
				data["gestureId"] = sym_t::Number(number_t::Long(e.dgesture.gestureId));
				data["numFingers"] = sym_t::Number(number_t::Long(e.dgesture.numFingers));
				data["error"] = sym_t::Number(number_t::Double(e.dgesture.error));
				data["x"] = sym_t::Number(number_t::Double(e.dgesture.x));
				data["y"] = sym_t::Number(number_t::Double(e.dgesture.y));
				return sym_t::Dictionary(data);
			case SDL_DROPFILE:
			case SDL_DROPBEGIN:
			case SDL_DROPTEXT:
			case SDL_DROPCOMPLETE:
				data["timestamp"] = sym_t::Number(number_t::Long(e.drop.timestamp));
				data["windowID"] = sym_t::Number(number_t::Long(e.drop.windowID));
				data["window"] = libsdl::registered[e.drop.windowID];
				if (e.drop.file != NULL) {
					data["file"] = sym_t::String(std::string(e.drop.file));
					SDL_free(e.drop.file);
				}
				return sym_t::Dictionary(data);
			default:
				return sym_t::Dictionary(data);
		}
	}

	data["type"] = sym_t::Number(number_t());
	return sym_t::Dictionary(data);
}

ROSSA_EXT_SIG(_window_register, args, token, hash, stack_trace)
{
	libsdl::registered[args[0].getNumber(token, stack_trace).getLong()] = args[1];
	return sym_t();
}

ROSSA_EXT_SIG(_window_getRenderer, args, token, hash, stack_trace)
{
	auto w = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Window);

	return sym_t::Pointer(w->getRenderer(token, stack_trace));
}

ROSSA_EXT_SIG(_renderer_draw, args, token, hash, stack_trace)
{
	auto g = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Renderer);

	int x = args[2].getNumber(token, stack_trace).getLong();
	int y = args[3].getNumber(token, stack_trace).getLong();

	g->addShape(args[1], x, y);
	return sym_t();
}

ROSSA_EXT_SIG(_shape_setColor, args, token, hash, stack_trace)
{
	auto shape = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Shape);

	libsdl::color_t r = args[1].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[2].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[3].getNumber(token, stack_trace).getLong();
	libsdl::color_t a = args[4].getNumber(token, stack_trace).getLong();

	shape->setColor(r, g, b, a);

	return sym_t();
}

ROSSA_EXT_SIG(_rotatable_setAngle, args, token, hash, stack_trace)
{
	auto rot = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Rotatable);

	int angle = args[1].getNumber(token, stack_trace).getDouble();

	rot->setAngle(angle);

	return sym_t();
}

ROSSA_EXT_SIG(_rotatable_setCenter, args, token, hash, stack_trace)
{
	auto rot = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Rotatable);

	int x = args[1].getNumber(token, stack_trace).getLong();
	int y = args[2].getNumber(token, stack_trace).getLong();

	rot->setCenter(x, y);

	return sym_t();
}

ROSSA_EXT_SIG(_rotatable_deCenter, args, token, hash, stack_trace)
{
	auto rot = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Rotatable);

	rot->deCenter();

	return sym_t();
}

ROSSA_EXT_SIG(_rotatable_setClip, args, token, hash, stack_trace)
{
	auto rot = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Rotatable);

	int x = args[1].getNumber(token, stack_trace).getLong();
	int y = args[2].getNumber(token, stack_trace).getLong();
	int width = args[3].getNumber(token, stack_trace).getLong();
	int height = args[4].getNumber(token, stack_trace).getLong();

	rot->setClip(x, y, width, height);

	return sym_t();
}

ROSSA_EXT_SIG(_rotatable_deClip, args, token, hash, stack_trace)
{
	auto rot = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Rotatable);

	rot->deClip();

	return sym_t();
}

ROSSA_EXT_SIG(_rect_init, args, token, hash, stack_trace)
{
	int width = args[0].getNumber(token, stack_trace).getLong();
	int height = args[1].getNumber(token, stack_trace).getLong();

	libsdl::color_t r = args[2].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[3].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[4].getNumber(token, stack_trace).getLong();
	libsdl::color_t a = args[5].getNumber(token, stack_trace).getLong();

	auto rect = std::make_shared<libsdl::Rectangle>(width, height, r, g, b, a);
	return sym_t::Pointer(rect);
}

ROSSA_EXT_SIG(_sizable_setSize, args, token, hash, stack_trace)
{
	auto sizable = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Sizable);

	int width = args[1].getNumber(token, stack_trace).getLong();
	int height = args[2].getNumber(token, stack_trace).getLong();

	sizable->setSize(width, height);
	return sym_t();
}

ROSSA_EXT_SIG(_sizable_setWidth, args, token, hash, stack_trace)
{
	auto sizable = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Sizable);

	int width = args[1].getNumber(token, stack_trace).getLong();

	sizable->setWidth(width);
	return sym_t();
}

ROSSA_EXT_SIG(_sizable_setHeight, args, token, hash, stack_trace)
{
	auto sizable = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Sizable);

	int height = args[1].getNumber(token, stack_trace).getLong();

	sizable->setHeight(height);
	return sym_t();
}

ROSSA_EXT_SIG(_line_init, args, token, hash, stack_trace)
{
	int x2 = args[0].getNumber(token, stack_trace).getLong();
	int y2 = args[1].getNumber(token, stack_trace).getLong();

	libsdl::color_t r = args[2].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[3].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[4].getNumber(token, stack_trace).getLong();
	libsdl::color_t a = args[5].getNumber(token, stack_trace).getLong();

	auto line = std::make_shared<libsdl::Line>(x2, y2, r, g, b, a);
	return sym_t::Pointer(line);
}

ROSSA_EXT_SIG(_point_init, args, token, hash, stack_trace)
{
	libsdl::color_t r = args[0].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[1].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[2].getNumber(token, stack_trace).getLong();
	libsdl::color_t a = args[3].getNumber(token, stack_trace).getLong();

	auto point = std::make_shared<libsdl::Point>(r, g, b, a);
	return sym_t::Pointer(point);
}

ROSSA_EXT_SIG(_image_init_nokey, args, token, hash, stack_trace)
{
	std::string path = args[0].getString(token, stack_trace);

	auto image = std::make_shared<libsdl::Image>(path, token, stack_trace);
	return sym_t::Pointer(image);
}

ROSSA_EXT_SIG(_image_init_key, args, token, hash, stack_trace)
{
	std::string path = args[0].getString(token, stack_trace);
	libsdl::color_t r = args[1].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[2].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[3].getNumber(token, stack_trace).getLong();

	auto image = std::make_shared<libsdl::Image>(path, token, stack_trace, r, g, b);
	return sym_t::Pointer(image);
}

ROSSA_EXT_SIG(_texture_init, args, token, hash, stack_trace)
{
	auto image = args[0];

	int width = args[1].getNumber(token, stack_trace).getLong();
	int height = args[2].getNumber(token, stack_trace).getLong();

	libsdl::color_t r = args[3].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[4].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[5].getNumber(token, stack_trace).getLong();

	auto texture = std::make_shared<libsdl::Texture>(image, width, height, r, g, b);
	return sym_t::Pointer(texture);
}

ROSSA_EXT_SIG(_font_init, args, token, hash, stack_trace)
{
	std::string fpath = args[0].getString(token, stack_trace);
	int fsize = args[1].getNumber(token, stack_trace).getLong();

	auto font = std::make_shared<libsdl::Font>(fpath, fsize, token, stack_trace);
	return sym_t::Pointer(font);
}

ROSSA_EXT_SIG(_text_init, args, token, hash, stack_trace)
{
	auto font = args[0];
	std::string s = args[1].getString(token, stack_trace);

	libsdl::color_t r = args[2].getNumber(token, stack_trace).getLong();
	libsdl::color_t g = args[3].getNumber(token, stack_trace).getLong();
	libsdl::color_t b = args[4].getNumber(token, stack_trace).getLong();
	libsdl::color_t a = args[5].getNumber(token, stack_trace).getLong();

	auto text = std::make_shared<libsdl::Text>(font, s, token, stack_trace, r, g, b, a);
	return sym_t::Pointer(text);
}

ROSSA_EXT_SIG(_text_setText, args, token, hash, stack_trace)
{
	auto text = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Text);

	text->setText(args[1].getString(token, stack_trace), token, stack_trace);
	return sym_t();
}

ROSSA_EXT_SIG(_texture_setImage, args, token, hash, stack_trace)
{
	auto texture = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Texture);

	texture->setImage(args[1]);
	return sym_t();
}

ROSSA_EXT_SIG(_renderer_update, args, token, hash, stack_trace)
{
	auto g = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Renderer);

	g->draw(token, stack_trace);
	return sym_t();
}

ROSSA_EXT_SIG(_renderer_clear, args, token, hash, stack_trace)
{
	auto g = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Renderer);

	g->clearAll();
	return sym_t();
}

ROSSA_EXT_SIG(_renderer_flush, args, token, hash, stack_trace)
{
	auto g = COERCE_PTR(
		args[0].getPointer(token, stack_trace),
		libsdl::Renderer);

	g->draw(token, stack_trace);
	g->clearAll();
	return sym_t();
}

EXPORT_FUNCTIONS(libsdl)
{
	ADD_EXT(_event_poll);
	ADD_EXT(_font_init);
	ADD_EXT(_image_init_key);
	ADD_EXT(_image_init_nokey);
	ADD_EXT(_line_init);
	ADD_EXT(_point_init);
	ADD_EXT(_rect_init);
	ADD_EXT(_renderer_clear);
	ADD_EXT(_renderer_draw);
	ADD_EXT(_renderer_flush);
	ADD_EXT(_renderer_update);
	ADD_EXT(_rotatable_deCenter);
	ADD_EXT(_rotatable_deClip);
	ADD_EXT(_rotatable_setAngle);
	ADD_EXT(_rotatable_setCenter);
	ADD_EXT(_rotatable_setClip);
	ADD_EXT(_sdl_init);
	ADD_EXT(_sdl_quit);
	ADD_EXT(_shape_setColor);
	ADD_EXT(_sizable_setHeight);
	ADD_EXT(_sizable_setSize);
	ADD_EXT(_sizable_setWidth);
	ADD_EXT(_text_init);
	ADD_EXT(_text_setText);
	ADD_EXT(_texture_init);
	ADD_EXT(_texture_setImage);
	ADD_EXT(_window_getRenderer);
	ADD_EXT(_window_init);
	ADD_EXT(_window_register);
}