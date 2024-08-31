#include <nlohmann/json.hpp>

#ifndef JSON_CONVERTER_H
#define JSON_CONVERTER_H

using json = nlohmann::json;

class JsonConverter {
public:
    explicit JsonConverter(json& json) : m_json(json) {}

    template <typename TField>
    void operator()(TField&& field)
    {
        m_json[field.name()] = field.value();
    }

    template<typename TFieldBase, typename T, typename... TOptions>
    void operator()(const comms::field::IntValue<TFieldBase, T, TOptions...>& field)
    {
        m_json[field.name()] = field.value();
    }

    template<typename TFieldBase, typename TMembers, typename... TOptions>
    void operator()(const comms::field::Bitfield<TFieldBase, TMembers, TOptions...>& field)
    {
        json subObj;
        comms::util::tupleForEach(field.value(), JsonConverter(subObj));
        m_json[field.name()] = subObj;
    }

private:
    json& m_json;
};

#endif //JSON_CONVERTER_H
