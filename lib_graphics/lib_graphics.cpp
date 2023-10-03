#include "../main/mediator/mediator.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <regex>
#include <vector>
#include <string>

typedef unsigned long long hash_ull;

const std::string format(const std::string &fmt, const std::vector<std::string> &args)
{
	std::string in(fmt), out;		 // unformatted and formatted strings
	std::regex re_arg("\\{\\d+\\}"); // search for {0}, {1}, ...
	std::regex re_idx("\\d+");		 // search for 0, 1, ...
	std::smatch m_arg, m_idx;		 // store matches
	size_t idx = 0;					 // index of argument inside {...}

	// Replace all {x} with vbuf[x]
	while (std::regex_search(in, m_arg, re_arg))
	{
		out += m_arg.prefix();
		auto text = m_arg[0].str();
		if (std::regex_search(text, m_idx, re_idx))
		{
			idx = std::stoi(m_idx[0].str());
		}
		if (idx < args.size())
		{
			out += std::regex_replace(text, re_arg, args[idx]);
		}
		in = m_arg.suffix();
	}
	out += in;
	return out;
}

namespace lib_graphics
{
	typedef unsigned char color_t;

	struct Window;

	static std::map<Uint32, std::shared_ptr<Window>> registered = {};

	struct Image
	{
	private:
		SDL_Surface *loaded = NULL;
		SDL_Texture *image = NULL;

	public:
		Image(const std::string &path, const color_t &r, const color_t &g, const color_t &b)
		{
			loaded = IMG_Load(path.c_str());
			if (loaded == NULL)
				throw library_error_t(format("Texture file `{0}` loading error: {1}", {path, IMG_GetError()}));
			SDL_SetColorKey(loaded, SDL_TRUE, SDL_MapRGB(loaded->format, r, g, b));
		}

		Image(const std::string &path)
		{
			loaded = IMG_Load(path.c_str());
			if (loaded == NULL)
				throw library_error_t(format("Texture file `{0}` loading error: {1}", {path, IMG_GetError()}));
		}

		SDL_Texture *getImage(SDL_Renderer *renderer)
		{
			if (image == NULL)
			{
				image = SDL_CreateTextureFromSurface(renderer, loaded);
				if (image == NULL)
					throw library_error_t(format("Cannot create renderable image: {0}", {SDL_GetError()}));
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

		Font(const std::string &path, const int &size)
		{
			font = TTF_OpenFont(path.c_str(), size);
			if (font == NULL)
				throw library_error_t(format("Failure to initialize font: {0}", {TTF_GetError()}));
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
			: r{r}, g{g}, b{b}, a{a}, id{id_count++}
		{
		}

		virtual void draw(SDL_Renderer *renderer, const int &x, const int &y) = 0;

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
			: Shape(r, g, b, a), width{width}, height{height}
		{
		}

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
		{
		}

		void setAngle(const double &angle)
		{
			this->angle = angle;
		}

		void setCenter(const int &x, const int &y)
		{
			if (center != NULL)
				SDL_free(center);
			center = new SDL_Point({x, y});
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
			clip = new SDL_Rect({cx, cy, cwidth, cheight});
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
		{
		}

		void draw(SDL_Renderer *renderer, const int &x, const int &y) override
		{
			SDL_Rect temp = {x, y, width, height};
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw library_error_t(format("Error setting shape color: {0}", {SDL_GetError()}));
			if (SDL_RenderFillRect(renderer, &temp) < 0)
				throw library_error_t(format("Error drawing shape: {0}", {SDL_GetError()}));
		}
	};

	struct Line : public Shape
	{
		int x1;
		int y1;
		Line(const int &x1, const int &y1, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a), x1{x1}, y1{y1}
		{
		}

		void draw(SDL_Renderer *renderer, const int &x, const int &y) override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw library_error_t(format("Error setting shape color: {0}", {SDL_GetError()}));
			if (SDL_RenderDrawLine(renderer, x, y, x1, y1) < 0)
				throw library_error_t(format("Error drawing shape: {0}", {SDL_GetError()}));
		}
	};

	struct Point : public Shape
	{
		Point(const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a)
		{
		}

		void draw(SDL_Renderer *renderer, const int &x, const int &y) override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw library_error_t(format("Error setting shape color: {0}", {SDL_GetError()}));
			if (SDL_RenderDrawPoint(renderer, x, y) < 0)
				throw library_error_t(format("Error drawing shape: {0}", {SDL_GetError()}));
		}
	};

	struct Texture : public Rotatable
	{
		std::shared_ptr<Image> image;

		Texture(const std::shared_ptr<Image> &image, const int &width, const int &height, const color_t &r, const color_t &g, const color_t &b)
			: Rotatable(width, height, r, g, b, 0), image{image}
		{
		}

		void setImage(const std::shared_ptr<Image> &image)
		{
			this->image = image;
		}

		void draw(SDL_Renderer *renderer, const int &x, const int &y) override
		{
			auto img = image->getImage(renderer);
			SDL_SetTextureColorMod(img, r, g, b);
			SDL_Rect temp = {x, y, width, height};
			SDL_RenderCopyEx(renderer, img, clip, &temp, angle, center, SDL_FLIP_NONE);
		}
	};

	struct Renderer
	{
		Uint32 windowID;
		SDL_Renderer *renderer = NULL;
		std::vector<std::pair<std::shared_ptr<Shape>, std::pair<int, int>>> shapes;

		Renderer(SDL_Window *window)
		{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL)
				throw library_error_t(format("Failure to initialize renderer: {0}", {SDL_GetError()}));
		}

		void addShape(const std::shared_ptr<Shape> &shape, const int &x, const int &y)
		{
			shapes.push_back({shape, {x, y}});
		}

		void clearAll()
		{
			shapes.clear();
		}

		void draw()
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			for (auto &s : shapes)
			{
				s.first->draw(renderer, s.second.first, s.second.second);
			}
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
		std::shared_ptr<Font> font;

	public:
		Text(const std::shared_ptr<Font> &font, const std::string &s, const color_t &r, const color_t &g, const color_t &b, const color_t &a)
			: Shape(r, g, b, a)
		{
			this->font = font;
			setText(s);
		}

		SDL_Texture *renderFont(SDL_Renderer *renderer)
		{
			if (image == NULL)
			{
				image = SDL_CreateTextureFromSurface(renderer, loaded);
				if (image == NULL)
					throw library_error_t(format("Cannot create renderable image: {0}", {SDL_GetError()}));
				SDL_QueryTexture(image, NULL, NULL, &width, &height);
				SDL_FreeSurface(loaded);
				loaded = NULL;
			}
			return image;
		}

		void setText(const std::string &s)
		{
			if (s == text && loaded != NULL)
				return;
			freeData();
			if (s == "")
				return;
			auto fdata = font->font;
			loaded = TTF_RenderText_Solid(fdata, s.c_str(), {r, g, b, a});
			if (loaded == NULL)
				throw library_error_t(format("Font rendering error: {0}", {TTF_GetError()}));
			text = s;
		}

		void draw(SDL_Renderer *renderer, const int &x, const int &y) override
		{
			if (text == "")
				return;
			SDL_Rect temp = {x, y, width, height};
			SDL_RenderCopy(renderer, renderFont(renderer), NULL, &temp);
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

		Window(const std::string &title, const int &width, const int &height)
		{
			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
			if (window == NULL)
				throw library_error_t(format("Failure to initialize window: {0}", {SDL_GetError()}));

			this->windowID = SDL_GetWindowID(window);
		}

		std::shared_ptr<Renderer> getRenderer()
		{
			auto g = std::make_shared<Renderer>(window);
			return g;
		}

		~Window()
		{
			lib_graphics::registered.erase(windowID);
			SDL_DestroyWindow(window);
		}
	};
}

ROSSA_EXT_SIG(_sdl_init, args)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		throw library_error_t(format("Failure to initialize SDL: {0}", {SDL_GetError()}));

	int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
	if (!(IMG_Init(imgFlags) & imgFlags))
		throw library_error_t(format("Failure to initialize SDL_image: {0}", {IMG_GetError()}));

	if (TTF_Init() < 0)
		throw library_error_t(format("Failure to initialize SDL_ttf: {0}", {TTF_GetError()}));

	return mediator_t();
}

ROSSA_EXT_SIG(_sdl_quit, args)
{
	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	return mediator_t();
}

ROSSA_EXT_SIG(_window_init, args)
{
	auto v0 = COERCE_STRING(args[0]);
	auto v1 = COERCE_NUMBER(args[1]);
	auto v2 = COERCE_NUMBER(args[2]);

	auto w = std::make_shared<lib_graphics::Window>(v0, v1.getLong(), v2.getLong());
	std::vector<mediator_t> v = {
		mediator_t(MEDIATOR_POINTER, w),
		mediator_t(MEDIATOR_NUMBER, std::make_shared<number_t>(number_t::Long(w->windowID)))};
	return mediator_t(
		MEDIATOR_ARRAY,
		std::make_shared<std::vector<mediator_t>>(v));
}

ROSSA_EXT_SIG(_event_poll, args)
{
	SDL_Event e;
	std::map<const std::string, const mediator_t> data;
	if (SDL_PollEvent(&e))
	{
		data.insert({"type", mediator_t(
								 MEDIATOR_NUMBER,
								 std::make_shared<number_t>(number_t::Long(e.type)))});
		switch (e.type)
		{
		case SDL_WINDOWEVENT:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.window.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.window.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.window.windowID])});
			data.insert({"event", MAKE_NUMBER(number_t::Long(e.window.event))});
			data.insert({"data1", MAKE_NUMBER(number_t::Long(e.window.data1))});
			data.insert({"data2", MAKE_NUMBER(number_t::Long(e.window.data2))});
			return MAKE_DICTIONARY(data);
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.key.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.key.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.key.windowID])});
			data.insert({"state", MAKE_NUMBER(number_t::Long(e.key.state))});
			data.insert({"repeat", MAKE_NUMBER(number_t::Long(e.key.repeat))});
			std::map<const std::string, const mediator_t> keysym;
			keysym.insert({"scancode", MAKE_NUMBER(number_t::Long(e.key.keysym.scancode))});
			keysym.insert({"sym", MAKE_NUMBER(number_t::Long(e.key.keysym.sym))});
			keysym.insert({"mod", MAKE_NUMBER(number_t::Long(e.key.keysym.mod))});
			data.insert({"keysym", MAKE_DICTIONARY(keysym)});
			return MAKE_DICTIONARY(data);
		}
		case SDL_TEXTEDITING:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.edit.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.edit.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.edit.windowID])});
			data.insert({"text", MAKE_STRING(std::string(e.edit.text))});
			data.insert({"start", MAKE_NUMBER(number_t::Long(e.edit.start))});
			data.insert({"length", MAKE_NUMBER(number_t::Long(e.edit.length))});
			return MAKE_DICTIONARY(data);
		case SDL_TEXTINPUT:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.text.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.text.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.text.windowID])});
			data.insert({"text", MAKE_STRING(std::string(e.text.text))});
			return MAKE_DICTIONARY(data);
		case SDL_MOUSEMOTION:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.motion.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.motion.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.motion.windowID])});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.motion.which))});
			data.insert({"state", MAKE_NUMBER(number_t::Long(e.motion.state))});
			data.insert({"x", MAKE_NUMBER(number_t::Long(e.motion.x))});
			data.insert({"y", MAKE_NUMBER(number_t::Long(e.motion.y))});
			data.insert({"xrel", MAKE_NUMBER(number_t::Long(e.motion.xrel))});
			data.insert({"yrel", MAKE_NUMBER(number_t::Long(e.motion.yrel))});
			return MAKE_DICTIONARY(data);
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.button.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.button.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.button.windowID])});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.button.which))});
			data.insert({"state", MAKE_NUMBER(number_t::Long(e.button.state))});
			data.insert({"x", MAKE_NUMBER(number_t::Long(e.button.x))});
			data.insert({"y", MAKE_NUMBER(number_t::Long(e.button.y))});
			data.insert({"button", MAKE_NUMBER(number_t::Long(e.button.button))});
			data.insert({"clicks", MAKE_NUMBER(number_t::Long(e.button.clicks))});
			return MAKE_DICTIONARY(data);
		case SDL_MOUSEWHEEL:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.wheel.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.wheel.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.wheel.windowID])});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.wheel.which))});
			data.insert({"direction", MAKE_NUMBER(number_t::Long(e.wheel.direction))});
			data.insert({"x", MAKE_NUMBER(number_t::Long(e.wheel.x))});
			data.insert({"y", MAKE_NUMBER(number_t::Long(e.wheel.y))});
			return MAKE_DICTIONARY(data);
		case SDL_JOYAXISMOTION:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.jaxis.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.jaxis.which))});
			data.insert({"axis", MAKE_NUMBER(number_t::Long(e.jaxis.axis))});
			data.insert({"value", MAKE_NUMBER(number_t::Long(e.jaxis.value))});
			return MAKE_DICTIONARY(data);
		case SDL_JOYBALLMOTION:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.jball.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.jball.which))});
			data.insert({"ball", MAKE_NUMBER(number_t::Long(e.jball.ball))});
			data.insert({"xrel", MAKE_NUMBER(number_t::Long(e.jball.xrel))});
			data.insert({"yrel", MAKE_NUMBER(number_t::Long(e.jball.yrel))});
			return MAKE_DICTIONARY(data);
		case SDL_JOYHATMOTION:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.jhat.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.jhat.which))});
			data.insert({"hat", MAKE_NUMBER(number_t::Long(e.jhat.hat))});
			data.insert({"value", MAKE_NUMBER(number_t::Long(e.jhat.value))});
			return MAKE_DICTIONARY(data);
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.jbutton.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.jbutton.which))});
			data.insert({"button", MAKE_NUMBER(number_t::Long(e.jbutton.button))});
			data.insert({"state", MAKE_NUMBER(number_t::Long(e.jbutton.state))});
			return MAKE_DICTIONARY(data);
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.jdevice.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.jdevice.which))});
			return MAKE_DICTIONARY(data);
		case SDL_CONTROLLERAXISMOTION:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.caxis.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.caxis.which))});
			data.insert({"axis", MAKE_NUMBER(number_t::Long(e.caxis.axis))});
			data.insert({"value", MAKE_NUMBER(number_t::Long(e.caxis.value))});
			return MAKE_DICTIONARY(data);
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.cbutton.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.cbutton.which))});
			data.insert({"button", MAKE_NUMBER(number_t::Long(e.cbutton.button))});
			data.insert({"state", MAKE_NUMBER(number_t::Long(e.cbutton.state))});
			return MAKE_DICTIONARY(data);
		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEREMAPPED:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.cdevice.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.cdevice.which))});
			return MAKE_DICTIONARY(data);
		case SDL_AUDIODEVICEADDED:
		case SDL_AUDIODEVICEREMOVED:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.adevice.timestamp))});
			data.insert({"which", MAKE_NUMBER(number_t::Long(e.adevice.which))});
			data.insert({"iscapture", MAKE_NUMBER(number_t::Long(e.adevice.iscapture))});
			return MAKE_DICTIONARY(data);
		case SDL_QUIT:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.quit.timestamp))});
			return MAKE_DICTIONARY(data);
		case SDL_FINGERMOTION:
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.tfinger.timestamp))});
			data.insert({"touchId", MAKE_NUMBER(number_t::Long(e.tfinger.touchId))});
			data.insert({"fingerId", MAKE_NUMBER(number_t::Long(e.tfinger.fingerId))});
			data.insert({"x", MAKE_NUMBER(number_t::Double(e.tfinger.x))});
			data.insert({"y", MAKE_NUMBER(number_t::Double(e.tfinger.y))});
			data.insert({"dx", MAKE_NUMBER(number_t::Double(e.tfinger.dx))});
			data.insert({"dy", MAKE_NUMBER(number_t::Double(e.tfinger.dy))});
			data.insert({"pressure", MAKE_NUMBER(number_t::Double(e.tfinger.pressure))});
			return MAKE_DICTIONARY(data);
		case SDL_MULTIGESTURE:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.mgesture.timestamp))});
			data.insert({"touchId", MAKE_NUMBER(number_t::Long(e.mgesture.touchId))});
			data.insert({"dTheta", MAKE_NUMBER(number_t::Double(e.mgesture.dTheta))});
			data.insert({"dDist", MAKE_NUMBER(number_t::Double(e.mgesture.dDist))});
			data.insert({"x", MAKE_NUMBER(number_t::Double(e.mgesture.x))});
			data.insert({"y", MAKE_NUMBER(number_t::Double(e.mgesture.y))});
			data.insert({"numFingers", MAKE_NUMBER(number_t::Long(e.mgesture.numFingers))});
			return MAKE_DICTIONARY(data);
		case SDL_DOLLARGESTURE:
		case SDL_DOLLARRECORD:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.dgesture.timestamp))});
			data.insert({"touchId", MAKE_NUMBER(number_t::Long(e.dgesture.touchId))});
			data.insert({"gestureId", MAKE_NUMBER(number_t::Long(e.dgesture.gestureId))});
			data.insert({"numFingers", MAKE_NUMBER(number_t::Long(e.dgesture.numFingers))});
			data.insert({"error", MAKE_NUMBER(number_t::Double(e.dgesture.error))});
			data.insert({"x", MAKE_NUMBER(number_t::Double(e.dgesture.x))});
			data.insert({"y", MAKE_NUMBER(number_t::Double(e.dgesture.y))});
			return MAKE_DICTIONARY(data);
		case SDL_DROPFILE:
		case SDL_DROPBEGIN:
		case SDL_DROPTEXT:
		case SDL_DROPCOMPLETE:
			data.insert({"timestamp", MAKE_NUMBER(number_t::Long(e.drop.timestamp))});
			data.insert({"windowID", MAKE_NUMBER(number_t::Long(e.drop.windowID))});
			data.insert({"window", MAKE_POINTER(lib_graphics::registered[e.drop.windowID])});
			if (e.drop.file != NULL)
			{
				data.insert({"file", MAKE_STRING(std::string(e.drop.file))});
				SDL_free(e.drop.file);
			}
			return MAKE_DICTIONARY(data);
		default:
			return MAKE_DICTIONARY(data);
		}
	}

	data.insert({"type", MAKE_NUMBER(number_t())});
	return MAKE_DICTIONARY(data);
}

ROSSA_EXT_SIG(_window_register, args)
{
	auto v0 = COERCE_NUMBER(args[0]);
	auto v1 = COERCE_POINTER(args[1], lib_graphics::Window);

	lib_graphics::registered[v0.getLong()] = v1;
	return mediator_t();
}

ROSSA_EXT_SIG(_window_getRenderer, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Window);

	return MAKE_POINTER(v0->getRenderer());
}

ROSSA_EXT_SIG(_renderer_draw, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Renderer);
	auto v1 = COERCE_POINTER(args[1], lib_graphics::Shape);
	auto v2 = COERCE_NUMBER(args[2]);
	auto v3 = COERCE_NUMBER(args[3]);

	int x = v2.getLong();
	int y = v3.getLong();

	v0->addShape(v1, x, y);
	return mediator_t();
}

ROSSA_EXT_SIG(_shape_setColor, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Shape);

	lib_graphics::color_t r = COERCE_NUMBER(args[1]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[2]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[3]).getLong();
	lib_graphics::color_t a = COERCE_NUMBER(args[4]).getLong();

	v0->setColor(r, g, b, a);

	return mediator_t();
}

ROSSA_EXT_SIG(_rotatable_setAngle, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Rotatable);

	int angle = COERCE_NUMBER(args[1]).getDouble();

	v0->setAngle(angle);

	return mediator_t();
}

ROSSA_EXT_SIG(_rotatable_setCenter, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Rotatable);

	int x = COERCE_NUMBER(args[1]).getLong();
	int y = COERCE_NUMBER(args[2]).getLong();

	v0->setCenter(x, y);

	return mediator_t();
}

ROSSA_EXT_SIG(_rotatable_deCenter, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Rotatable);

	v0->deCenter();

	return mediator_t();
}

ROSSA_EXT_SIG(_rotatable_setClip, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Rotatable);

	int x = COERCE_NUMBER(args[1]).getLong();
	int y = COERCE_NUMBER(args[2]).getLong();
	int width = COERCE_NUMBER(args[3]).getLong();
	int height = COERCE_NUMBER(args[4]).getLong();

	v0->setClip(x, y, width, height);

	return mediator_t();
}

ROSSA_EXT_SIG(_rotatable_deClip, args)
{
	auto v0 = COERCE_POINTER(args[0], lib_graphics::Rotatable);

	v0->deClip();

	return mediator_t();
}

ROSSA_EXT_SIG(_rect_init, args)
{
	int width = COERCE_NUMBER(args[0]).getLong();
	int height = COERCE_NUMBER(args[1]).getLong();

	lib_graphics::color_t r = COERCE_NUMBER(args[2]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[3]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[4]).getLong();
	lib_graphics::color_t a = COERCE_NUMBER(args[5]).getLong();

	auto rect = std::make_shared<lib_graphics::Rectangle>(width, height, r, g, b, a);
	return MAKE_POINTER(rect);
}

ROSSA_EXT_SIG(_sizable_setSize, args)
{
	auto sizable = COERCE_POINTER(args[0], lib_graphics::Sizable);

	int width = COERCE_NUMBER(args[1]).getLong();
	int height = COERCE_NUMBER(args[2]).getLong();

	sizable->setSize(width, height);
	return mediator_t();
}

ROSSA_EXT_SIG(_sizable_setWidth, args)
{
	auto sizable = COERCE_POINTER(args[0], lib_graphics::Sizable);

	int width = COERCE_NUMBER(args[1]).getLong();

	sizable->setWidth(width);
	return mediator_t();
}

ROSSA_EXT_SIG(_sizable_setHeight, args)
{
	auto sizable = COERCE_POINTER(args[0], lib_graphics::Sizable);

	int height = COERCE_NUMBER(args[1]).getLong();

	sizable->setHeight(height);
	return mediator_t();
}

ROSSA_EXT_SIG(_line_init, args)
{
	int x2 = COERCE_NUMBER(args[0]).getLong();
	int y2 = COERCE_NUMBER(args[1]).getLong();

	lib_graphics::color_t r = COERCE_NUMBER(args[2]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[3]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[4]).getLong();
	lib_graphics::color_t a = COERCE_NUMBER(args[5]).getLong();

	auto line = std::make_shared<lib_graphics::Line>(x2, y2, r, g, b, a);
	return MAKE_POINTER(line);
}

ROSSA_EXT_SIG(_point_init, args)
{
	lib_graphics::color_t r = COERCE_NUMBER(args[0]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[1]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[2]).getLong();
	lib_graphics::color_t a = COERCE_NUMBER(args[3]).getLong();

	auto point = std::make_shared<lib_graphics::Point>(r, g, b, a);
	return MAKE_POINTER(point);
}

ROSSA_EXT_SIG(_image_init_nokey, args)
{
	std::string path = COERCE_STRING(args[0]);

	auto image = std::make_shared<lib_graphics::Image>(path);
	return MAKE_POINTER(image);
}

ROSSA_EXT_SIG(_image_init_key, args)
{
	std::string path = COERCE_STRING(args[0]);
	lib_graphics::color_t r = COERCE_NUMBER(args[1]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[2]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[3]).getLong();

	auto image = std::make_shared<lib_graphics::Image>(path, r, g, b);
	return MAKE_POINTER(image);
}

ROSSA_EXT_SIG(_texture_init, args)
{
	auto image = COERCE_POINTER(args[0], lib_graphics::Image);

	int width = COERCE_NUMBER(args[1]).getLong();
	int height = COERCE_NUMBER(args[2]).getLong();

	lib_graphics::color_t r = COERCE_NUMBER(args[3]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[4]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[5]).getLong();

	auto texture = std::make_shared<lib_graphics::Texture>(image, width, height, r, g, b);
	return MAKE_POINTER(texture);
}

ROSSA_EXT_SIG(_font_init, args)
{
	std::string fpath = COERCE_STRING(args[0]);
	int fsize = COERCE_NUMBER(args[1]).getLong();

	auto font = std::make_shared<lib_graphics::Font>(fpath, fsize);
	return MAKE_POINTER(font);
}

ROSSA_EXT_SIG(_text_init, args)
{
	auto font = COERCE_POINTER(args[0], lib_graphics::Font);
	std::string s = COERCE_STRING(args[1]);

	lib_graphics::color_t r = COERCE_NUMBER(args[2]).getLong();
	lib_graphics::color_t g = COERCE_NUMBER(args[3]).getLong();
	lib_graphics::color_t b = COERCE_NUMBER(args[4]).getLong();
	lib_graphics::color_t a = COERCE_NUMBER(args[5]).getLong();

	auto text = std::make_shared<lib_graphics::Text>(font, s, r, g, b, a);
	return MAKE_POINTER(text);
}

ROSSA_EXT_SIG(_text_setText, args)
{
	auto text = COERCE_POINTER(args[0], lib_graphics::Text);

	text->setText(COERCE_STRING(args[1]));
	return mediator_t();
}

ROSSA_EXT_SIG(_texture_setImage, args)
{
	auto texture = COERCE_POINTER(args[0], lib_graphics::Texture);

	texture->setImage(COERCE_POINTER(args[1], lib_graphics::Image));
	return mediator_t();
}

ROSSA_EXT_SIG(_renderer_update, args)
{
	auto g = COERCE_POINTER(args[0], lib_graphics::Renderer);

	g->draw();
	return mediator_t();
}

ROSSA_EXT_SIG(_renderer_clear, args)
{
	auto g = COERCE_POINTER(args[0], lib_graphics::Renderer);

	g->clearAll();
	return mediator_t();
}

ROSSA_EXT_SIG(_renderer_flush, args)
{
	auto g = COERCE_POINTER(args[0], lib_graphics::Renderer);

	g->draw();
	g->clearAll();
	return mediator_t();
}

EXPORT_FUNCTIONS(lib_graphics)
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