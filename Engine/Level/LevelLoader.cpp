#include "LevelLoader.h"
#include "../Tile/TileLayer.h"
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <sstream>

namespace Engine {

std::vector<Entity*> LevelLoader::loadFromJSON(const std::string& path, TileLayer* outTiles) {
    std::vector<Entity*> entities;
    m_error.clear();

    std::ifstream file(path);
    if (!file) { m_error = "Failed to open: " + path; return entities; }

    std::stringstream buf;
    buf << file.rdbuf();

    rapidjson::Document doc;
    doc.Parse(buf.str().c_str());
    if (doc.HasParseError()) { m_error = "JSON parse error"; return entities; }

    m_width = 800.f;
    m_height = 600.f;
    if (doc.HasMember("width") && doc["width"].IsNumber()) m_width = doc["width"].GetFloat();
    if (doc.HasMember("height") && doc["height"].IsNumber()) m_height = doc["height"].GetFloat();

    if (!doc.HasMember("entities") || !doc["entities"].IsArray()) { m_error = "Missing entities array"; return entities; }

    for (auto& j : doc["entities"].GetArray()) {
        std::string typeStr = "default";
        if (j.HasMember("type") && j["type"].IsString()) typeStr = j["type"].GetString();

        Entity* e = m_factory ? m_factory->create(typeStr) : new Entity();
        if (j.HasMember("name") && j["name"].IsString()) e->name = j["name"].GetString();
        e->type = typeStr;

        if (j.HasMember("position") && j["position"].IsObject()) {
            auto& p = j["position"];
            if (p.HasMember("x") && p["x"].IsNumber()) e->position.x = p["x"].GetFloat();
            if (p.HasMember("y") && p["y"].IsNumber()) e->position.y = p["y"].GetFloat();
        }
        if (j.HasMember("size") && j["size"].IsObject()) {
            auto& s = j["size"];
            if (s.HasMember("x") && s["x"].IsNumber()) e->size.x = s["x"].GetFloat();
            if (s.HasMember("y") && s["y"].IsNumber()) e->size.y = s["y"].GetFloat();
        }
        if (j.HasMember("velocity") && j["velocity"].IsObject()) {
            auto& v = j["velocity"];
            if (v.HasMember("x") && v["x"].IsNumber()) e->velocity.x = v["x"].GetFloat();
            if (v.HasMember("y") && v["y"].IsNumber()) e->velocity.y = v["y"].GetFloat();
        }
        if (j.HasMember("color") && j["color"].IsObject()) {
            auto& c = j["color"];
            e->color = sf::Color(
                (c.HasMember("r") && c["r"].IsNumber()) ? c["r"].GetInt() : 255,
                (c.HasMember("g") && c["g"].IsNumber()) ? c["g"].GetInt() : 255,
                (c.HasMember("b") && c["b"].IsNumber()) ? c["b"].GetInt() : 255,
                (c.HasMember("a") && c["a"].IsNumber()) ? c["a"].GetInt() : 255);
        }
        if (j.HasMember("isStatic") && j["isStatic"].IsBool()) e->isStatic = j["isStatic"].GetBool();
        if (j.HasMember("hasGravity") && j["hasGravity"].IsBool()) e->hasGravity = j["hasGravity"].GetBool();
        if (j.HasMember("isTrigger") && j["isTrigger"].IsBool()) e->isTrigger = j["isTrigger"].GetBool();
        if (j.HasMember("texturePath") && j["texturePath"].IsString())
            e->texturePath = j["texturePath"].GetString();

        if (j.HasMember("properties") && j["properties"].IsObject()) {
            Entity::Properties props;
            Entity::StringProperties stringProps;
            for (auto& m : j["properties"].GetObject()) {
                if (m.value.IsNumber())
                    props[m.name.GetString()] = m.value.GetFloat();
                else if (m.value.IsString())
                    stringProps[m.name.GetString()] = m.value.GetString();
            }
            if (!props.empty()) e->deserializeProperties(props);
            if (!stringProps.empty()) e->deserializeStringProperties(stringProps);
        }
        entities.push_back(e);
    }

    if (outTiles) {
        outTiles->resize(0, 0, outTiles->cellSize() > 0.f ? outTiles->cellSize() : 32.f);
        if (doc.HasMember("tiles") && doc["tiles"].IsObject()) {
            const auto& t = doc["tiles"];
            int tw = (t.HasMember("width")  && t["width"].IsInt())    ? t["width"].GetInt()    : 0;
            int th = (t.HasMember("height") && t["height"].IsInt())   ? t["height"].GetInt()   : 0;
            float cs = (t.HasMember("cellSize") && t["cellSize"].IsNumber()) ? t["cellSize"].GetFloat() : 32.f;
            if (tw > 0 && th > 0 && cs > 0.f) {
                outTiles->resize(tw, th, cs);
                if (t.HasMember("cells") && t["cells"].IsArray()) {
                    const auto& arr = t["cells"].GetArray();
                    auto& cells = outTiles->cells();
                    size_t n = std::min<size_t>(cells.size(), arr.Size());
                    for (size_t i = 0; i < n; ++i)
                        if (arr[(rapidjson::SizeType)i].IsInt())
                            cells[i] = arr[(rapidjson::SizeType)i].GetInt();
                }
            }
        }
    }
    return entities;
}

bool LevelLoader::saveToJSON(const std::string& path, const std::vector<Entity*>& entities, float width, float height, const TileLayer* tiles) {
    m_error.clear();
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    rapidjson::Value arr(rapidjson::kArrayType);
    for (auto* e : entities) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("name", rapidjson::Value(e->name.c_str(), alloc), alloc);
        obj.AddMember("type", rapidjson::Value(e->type.c_str(), alloc), alloc);

        rapidjson::Value pos(rapidjson::kObjectType);
        pos.AddMember("x", e->position.x, alloc);
        pos.AddMember("y", e->position.y, alloc);
        obj.AddMember("position", pos, alloc);

        rapidjson::Value sz(rapidjson::kObjectType);
        sz.AddMember("x", e->size.x, alloc);
        sz.AddMember("y", e->size.y, alloc);
        obj.AddMember("size", sz, alloc);

        rapidjson::Value vel(rapidjson::kObjectType);
        vel.AddMember("x", e->velocity.x, alloc);
        vel.AddMember("y", e->velocity.y, alloc);
        obj.AddMember("velocity", vel, alloc);

        rapidjson::Value col(rapidjson::kObjectType);
        col.AddMember("r", (int)e->color.r, alloc);
        col.AddMember("g", (int)e->color.g, alloc);
        col.AddMember("b", (int)e->color.b, alloc);
        col.AddMember("a", (int)e->color.a, alloc);
        obj.AddMember("color", col, alloc);

        obj.AddMember("isStatic", e->isStatic, alloc);
        obj.AddMember("hasGravity", e->hasGravity, alloc);
        obj.AddMember("isTrigger", e->isTrigger, alloc);
        if (!e->texturePath.empty())
            obj.AddMember("texturePath", rapidjson::Value(e->texturePath.c_str(), alloc), alloc);

        auto props = e->serializeProperties();
        auto stringProps = e->serializeStringProperties();
        if (!props.empty() || !stringProps.empty()) {
            rapidjson::Value propsObj(rapidjson::kObjectType);
            for (auto& [key, val] : props) {
                rapidjson::Value k(key.c_str(), alloc);
                rapidjson::Value v(val);
                propsObj.AddMember(k, v, alloc);
            }
            for (auto& [key, val] : stringProps) {
                rapidjson::Value k(key.c_str(), alloc);
                rapidjson::Value v(val.c_str(), alloc);
                propsObj.AddMember(k, v, alloc);
            }
            obj.AddMember("properties", propsObj, alloc);
        }
        arr.PushBack(obj, alloc);
    }
    doc.AddMember("entities", arr, alloc);
    doc.AddMember("width", width, alloc);
    doc.AddMember("height", height, alloc);

    if (tiles && tiles->width() > 0 && tiles->height() > 0) {
        rapidjson::Value tileObj(rapidjson::kObjectType);
        tileObj.AddMember("width", tiles->width(), alloc);
        tileObj.AddMember("height", tiles->height(), alloc);
        tileObj.AddMember("cellSize", tiles->cellSize(), alloc);
        rapidjson::Value cellArr(rapidjson::kArrayType);
        cellArr.Reserve((rapidjson::SizeType)tiles->cells().size(), alloc);
        for (int v : tiles->cells()) cellArr.PushBack(v, alloc);
        tileObj.AddMember("cells", cellArr, alloc);
        doc.AddMember("tiles", tileObj, alloc);
    }

    std::ofstream file(path);
    if (!file) { m_error = "Failed to write: " + path; return false; }

    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    doc.Accept(writer);
    file << sb.GetString();
    return true;
}

}
