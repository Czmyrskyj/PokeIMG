
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wattributes"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma GCC diagnostic pop

#include <print>
#include <iostream>
#include <string_view>
#include <random>
#include <stdexcept>
#include <memory>
enum class language : unsigned int{
	english,
	japanese,
	german,
};
std::string_view header_trans[] = {
	"DATA RECIEVED",
	"データ受信完了",
	"DATENEMPFANG ABGESCHLOSSEN",
};
std::string_view id_trans[] = {
	"Id",
	"アイデイ",
	"Id",
};
std::string_view name_trans[] = {
	"Name",
	"名前",
	"Name",
};
std::string_view type_trans[] = {
	"Type",
	"タイプ",
	"Typ"
};
std::string_view description_trans[] = {
	"Description",
	"説明",
	"Beschreibung",
};
int rand(int minval, int maxval){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<>dist(minval, maxval);
	return dist(gen);
}
struct stbi_deleter{
	void operator()(unsigned char* p) { stbi_image_free(p); }
};
using stbi_ptr = std::unique_ptr<unsigned char[], stbi_deleter>;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
namespace json = boost::json;
using tcp = net::ip::tcp;
stbi_ptr recieve_img(const std::string& url, int& width, int& height, int& channels){
    const std::string host = url.substr(8, url.find("/", 8) - 8);
    const std::string target = url.substr(url.find("/", 8));
    const std::string port = "443";
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    ctx.set_default_verify_paths();
    tcp::resolver resolver(ioc);
    ssl::stream<tcp::socket> stream{ioc, ctx};
    auto const endpoint = resolver.resolve(host, port);
    net::connect(stream.lowest_layer(), endpoint);
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data())){
        throw std::runtime_error("failed to set host name");
    }
    stream.handshake(ssl::stream<tcp::socket>::client);
    http::request<http::empty_body> req(http::verb::get, target, 11);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::vector_body<unsigned char>> res;
    http::read(stream, buffer, res);
    auto& body = res.body();
    unsigned char* img_data = stbi_load_from_memory(body.data(), body.size(), &width, &height, &channels, 0);
    if (!img_data){
	throw std::runtime_error("failed to load image");
    }
    return stbi_ptr(img_data);
}
int main(int argc, char* argv[]){
    if (argc < 2){
        std::println(stderr, "usage: pokeimg </pokemon-name or id/>");
        return -1;
    }
    static constexpr std::string_view symbolic = " .,-~*#&?$%@";
    static constexpr std::string_view numeric = " 0123456789";
    static constexpr std::string_view alphabetic = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    auto chars = std::string(symbolic);
    bool colorful = true;
    bool description = true;
    bool random = false;
    bool dream = false;
    auto lang = language::english;
    std::vector<std::string> flags(argc - 2);
    if (!flags.empty()){
	for (std::size_t i = 1; i < argc - 1; i++){
		flags[i - 1] = std::string(argv[i]);
	}
    	for (const auto& f : flags){
		if (f == "--nocol"){
			colorful = false;
		}
		if (f == "--nodesc"){
			description = false;
		}
		if (f == "--symbolic"){
			chars = std::string(symbolic);
		}
		if (f == "--numeric"){
			chars = std::string(numeric);
		}
		if (f == "--alphabetic"){
			chars = std::string(alphabetic);
		}
		if (f == "--dream"){
			dream = true;
		}
		if (f == "--en"){
			lang = language::english;
		}
		if (f == "--ja"){
			lang = language::japanese;
		}
		if (f == "--de"){
			lang = language::german;
		}
	}
    }
    if (std::string(argv[1]) == "--help"){
	std::println("usage: pokeimg </pokemon-name or id/>");
	std::println("flags:");
	std::println("--help: write all commands and flags");
	std::println("--nocol: display colorless sprite");
	std::println("--nodesc: provide no description");
	std::println("--symbolic: display symbolic sprite");
	std::println("--numeric: display numeric sprite");
	std::println("--alphabetic: display alphabetic sprite");
	std::println("--dream: display dream sprite");
	std::println("--en: enable english");
	std::println("--ja: enable japanese [日本語を有効にする]");
	std::println("--de: enable german [Deutsch aktivieren]");
	std::println("special:");
	std::println("@random: get random pokemon. [[pokeimg @random]]");
	return -1;
    }
    try {
        int width, height, channels;
        static constexpr std::string_view host = "pokeapi.co";
        static constexpr std::string_view port = "443";
        std::string target = std::format("/api/v2/pokemon/{}",(std::string(argv[argc - 1]) != "@random") ? std::string(argv[argc - 1]) : std::to_string(rand(1, 1025)));
        constexpr int version = 11;
        net::io_context ioc;
        ssl::context ctx{ssl::context::tlsv12_client};
        ctx.set_default_verify_paths();
        tcp::resolver resolver(ioc);
        ssl::stream<tcp::socket> stream{ioc, ctx};
        auto const endpoint = resolver.resolve(host, port);
        net::connect(stream.lowest_layer(), endpoint);
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data())){
            std::println(stderr, "failed to set host name");
            return -1;
        }
        stream.handshake(ssl::stream<tcp::socket>::client);
        http::request<http::string_body> req(http::verb::get, target, version);
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::write(stream, req);
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);
        if (res.result() != http::status::ok){
		std::println(stderr, "error: pokemon not found (HTTP {})", res.result_int());
		return -1;
	}
	std::string body = beast::buffers_to_string(res.body().data());
        json::value parsed_json = json::parse(body);
        if (parsed_json.is_object()){
            auto parsed_object = parsed_json.as_object();
	    if (description){
		    auto species_url = std::string(parsed_object.at("species").as_object().at("url").as_string());
		    if (species_url.front() == '"'){
			species_url = species_url.substr(1, species_url.size() - 1);
		    }
		    auto species_target = species_url.substr(species_url.find("/api/v2/"));
		    http::request<http::string_body> s_req(http::verb::get, species_target, 11);
		    s_req.set(http::field::host, host);
		    s_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		    http::write(stream, s_req);
		    beast::flat_buffer s_buffer;
		    http::response<http::dynamic_body> s_res;
		    http::read(stream, s_buffer, s_res);
		    std::string species_body = beast::buffers_to_string(s_res.body().data());
		    json::value species_json = json::parse(species_body);
		    auto species_object = species_json.as_object();
		    auto type_url = std::string(parsed_object.at("types").as_array().at(0).as_object().at("type").as_object().at("url").as_string());
		    auto type_target = type_url.substr(type_url.find("/api/v2/"));
		    http::request<http::string_body> t_req(http::verb::get, type_target, 11);
		    t_req.set(http::field::host, host);
		    t_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		    http::write(stream, t_req);
		    beast::flat_buffer t_buffer;
		    http::response<http::dynamic_body> t_res;
		    http::read(stream, t_buffer, t_res);
		    std::string type_body = beast::buffers_to_string(t_res.body().data());
		    json::value type_json = json::parse(type_body);
		    auto type_object = type_json.as_object(); 
		    std::string localized_type = "unknown";
		    for (auto& e : type_object.at("names").as_array()){
			auto obj = e.as_object();
			std::string lang_name = std::string(obj.at("language").as_object().at("name").as_string());
			if ((lang == language::english && lang_name == "en") || (lang == language::japanese && lang_name == "ja") || (lang == language::german && lang_name == "de")){
				localized_type = std::string(obj.at("name").as_string());
				break;
			}
		    }
		    std::string localized_name = "unknown";
		    auto names_arr = species_object.at("names").as_array();
		    for (auto& n : names_arr){
			auto obj = n.as_object();
			std::string lang_name = std::string(obj.at("language").as_object().at("name").as_string());
			if ((lang == language::english && lang_name == "en") || (lang == language::japanese && lang_name == "ja-hrkt") || (lang == language::german && lang_name == "de")){
				localized_name = std::string(obj.at("name").as_string());	
				break;
			}
		    }
		    std::string localized_description = "unknown";
		    auto description_arr = species_object.at("flavor_text_entries").as_array();
		    for (auto& d : description_arr){
			auto obj = d.as_object();
			std::string lang_name = std::string(obj.at("language").as_object().at("name").as_string());
			if ((lang == language::english && lang_name == "en") || (lang == language::japanese && lang_name == "ja-hrkt") || (lang == language::german && lang_name == "de")){
				localized_description = std::string(obj.at("flavor_text").as_string());	
				break;
			}
		    }
		    std::println("||===||{}||===", header_trans[static_cast<unsigned int>(lang)]);
		    int id = static_cast<int>(parsed_object.at("id").as_int64());
		    std::println("||{}: {}", id_trans[static_cast<unsigned int>(lang)], id);
		    std::println("||{}: {}", name_trans[static_cast<unsigned int>(lang)], localized_name);
		    std::println("||{}: {}", type_trans[static_cast<unsigned int>(lang)], localized_type);
		    std::println("||{}: {}", description_trans[static_cast<unsigned int>(lang)], localized_description);
	    }
	    std::string sprite_url;
	    if (!dream){
	    	sprite_url = std::string(parsed_object.at("sprites").as_object().at("front_default").as_string());
	    } else {
		    auto other = parsed_object.at("sprites").as_object().at("other").as_object();
		    auto dream_obj = other.at("dream_world").as_object();
		    if (dream_obj.at("front_default").is_null()){
			    std::println(stderr, "[warning]: dream world sprite not found, falling back to default.");
	    		    sprite_url = std::string(parsed_object.at("sprites").as_object().at("front_default").as_string());
		    } else {
			    auto raw_svg_url = std::string(dream_obj.at("front_default").as_string());
			    sprite_url = std::format("https://wsrv.nl/?url={}&output=png&w=64&il=3&trim=10", raw_svg_url);	
		    }
	    }
	    auto img = recieve_img(sprite_url, width, height, channels);
	    int max_x = 0; int min_x = width;
	    int max_y = 0; int min_y = height;
	    bool vis_pix = false;
	    for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			if (img[(y * width + x) * channels + (channels - 1)] > 0){
				if (x < min_x) min_x = x;
				if (x > max_x) max_x = x;
				if (y < min_y) min_y = y;
				if (y > max_y) max_y = y;
				vis_pix = true;
			}
		}
	    }
	    if (vis_pix){
		    for (std::size_t y = min_y; y <= max_y; y++){
			    for (std::size_t x = min_x; x <= max_x; x++){
				    std::size_t index = (y * width + x) * channels;
				    unsigned char r = img[index];
				    unsigned char g = img[index + 1];
				    unsigned char b = img[index + 2];
				    unsigned char a = (channels == 4) ? img[index + 3] : 255;
				    if (a < 127){
					    std::print("  ");
					    continue;
				    }
				    float brightness = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;
				    std::size_t i = static_cast<std::size_t>(brightness * (chars.size() - 1));
				    if (colorful) {
					    std::print("\033[38;2;{};{};{}m{}{}\033[0m", r, g, b, chars[i], chars[i]);
				    } else {
					    std::print("{}{}", chars[i], chars[i]);
				    }
			    }
			    std::println("");
		    }
	    }
	}
    } catch (const boost::system::system_error& e) {
	    std::println(stderr, "[system_error]: {} (Check your WI-FI connection)", e.what());
	    return -1;
    } catch (const std::exception& e) {
	    std::println(stderr, "[error]: {}", e.what());
	    return -1;
    }
    return 0;
} 
