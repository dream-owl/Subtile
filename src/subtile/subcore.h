#ifndef SUBTILE_SUBCORE_H
#define SUBTILE_SUBCORE_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <cassert>

class stSettings
{
public:
    static constexpr int32_t LifetimeIsland = 128;

    static constexpr int32_t SizeIsland = 4;
    static constexpr int32_t SizeRegion = 128;

    static constexpr uint8_t StoragePackage = 128;
    static constexpr uint8_t StorageIsland = 64;
    static constexpr uint8_t StorageLabel = 12;

    static constexpr size_t PoolMaterial = 4096;
    static constexpr size_t PoolBehavior = 2048;
    static constexpr size_t PoolIsland = 64;
};

class stException final : public std::exception
{
public:
    stException(std::string content) : m_content(content) {}
   ~stException() {}

    char const* what() const throw() { return m_content.c_str(); }

private:
    std::string m_content;
};

class stValue
{
public:
    stValue()=default;
    stValue(int16_t value) : value(value) {}

    void operator = (stValue const& right) { value = right.value; }
    void operator = (int16_t right) { value = right; }

    int16_t value;
};

class stValueMeta
{
public:
    constexpr stValueMeta()=default;
    constexpr stValueMeta(char const* name, int16_t index, int16_t limit) : name(name) , index(index) , limit(limit)  {}

    char const* const name;
    int16_t const index;
    int16_t const limit;
};

class stLabel
{
public:
    stLabel() : m_data { '\0' } {}

    stLabel(std::string_view const& stringView) {
        assert(stringView.size() < stSettings::StorageLabel);
        std::strcpy(m_data, stringView.data());
    }

    stLabel(char const* cString) {
        assert(std::strlen(cString) < stSettings::StorageLabel);
        std::strcpy(m_data, cString);
    }

    char const* str() const { return m_data; }

    bool operator == (stLabel const& right) const { return std::strcmp(m_data, right.m_data) == 0; }
    bool operator != (stLabel const& right) const { return std::strcmp(m_data, right.m_data) != 0; }

private:
    char m_data[stSettings::StorageLabel];
};

class stVector
{
public:
    stVector()=default;
    stVector(float x) : x(x) , y(x) {}
    stVector(float x, float y) : x(x) , y(y) {}

    float x;
    float y;
};

class stRadians
{
public:
    stRadians()=default;
    stRadians(float a) : a(a) {}

    float a;
};

class stTransform
{
public:
    stTransform()=default;
    stTransform(int32_t a, float x, float y, float radians) : altitude(a) , position(x, y) , rotation(radians) {}

    int32_t altitude;
    stVector position;
    stRadians rotation;
};

class stBounds
{
public:
    stBounds()=default;
    stBounds(int32_t lowerA, float lowerX, float lowerY, int32_t upperA, float upperX, float upperY) : lowerAltitude(lowerA) , upperAltitude(upperA) ,
                                                                                                       lower(lowerX, lowerY) , upper(upperX, upperY) {}

    bool test(stVector const& vector) const {
        return (vector.x >= lower.x) && (vector.x <= upper.x) && (vector.y >= lower.y) && (vector.y <= upper.y);
    }

    bool test(stBounds const& bounds) const {
        return (lowerAltitude <= bounds.upperAltitude) &&
               (upperAltitude >= bounds.lowerAltitude) &&
               (lower.x <= bounds.upper.x) && (lower.y <= bounds.upper.y) &&
               (upper.x >= bounds.lower.x) && (upper.y >= bounds.lower.y);
    }

    int32_t lowerAltitude;
    int32_t upperAltitude;
    stVector lower;
    stVector upper;
};

class stPackage
{
public:
    stPackage() : m_data{0} , m_size(0) {}
    stPackage(stPackage const&)=delete;

    template<class Struct>
    void encode(std::basic_string_view<char> const& name, Struct const* data, uint8_t const elements) {
        assert((name.size() + sizeof(Struct) * elements + sizeof(uint8_t) * 2) < stSettings::StoragePackage);

        uint8_t const nameSize = name.size();

        if(!nameSize)
            throw stException("stPackage::encode");

        std::memcpy(&m_data[0], &nameSize, sizeof(uint8_t));
        std::memcpy(&m_data[1], &name[0], nameSize);

        uint8_t const dataPart = sizeof(uint8_t) + nameSize;
        uint8_t const dataSize = sizeof(Struct) * elements;

        std::memcpy(&m_data[dataPart+0], &elements, sizeof(uint8_t));
        std::memcpy(&m_data[dataPart+1], data, dataSize);

        m_size = sizeof(uint8_t) * 2 + nameSize + dataSize;
    }

    template<class Struct>
    void decode(std::basic_string_view<char> const& name, Struct* data, uint8_t& elements) {
        char nameBuffer[64];
        uint8_t nameSize = 0;

        std::memcpy(&nameSize, &m_data[0], sizeof(uint8_t));
        std::memcpy(nameBuffer, &m_data[1], nameSize);

        if((nameSize != name.size()) || (name != nameBuffer))
            throw stException("stPackage::decode");

        uint8_t const dataPart = sizeof(uint8_t) + nameSize;
        std::memcpy(&elements, &m_data[dataPart+0], sizeof(uint8_t));

        uint8_t const dataSize = sizeof(Struct) * elements;
        std::memcpy(data, &m_data[dataPart+1], dataSize);

        m_size = sizeof(uint8_t) * 2 + nameSize + dataSize;
    }

    std::string_view name() const { return m_size ? "empty" : std::string_view(reinterpret_cast<char const*>(&m_data[1]), m_data[0]); }
    uint8_t const* data() const { return m_data; }
    uint8_t size() const { return m_size; }

private:
    uint8_t m_data[stSettings::StoragePackage];
    uint8_t m_size;
};

template<class Struct>
class stPool
{
public:
    stPool(int16_t capacity) : m_capacity(capacity) , m_poolSize(0) , m_freeSize(0) {
        m_pool.reset(new Struct[m_capacity]);
        m_free.reset(new int16_t[m_capacity]);
    }

    Struct& acquire(int16_t& handle) {
        assert(handle == 0);
        assert(m_freeSize || (m_poolSize < m_capacity));

        if(m_freeSize)
        {
            m_freeSize--;
            handle = m_free[m_freeSize];
        }
        else
        {
            handle = m_poolSize;
            m_poolSize++;
        }

        return m_pool[handle];
    }

    Struct& acquire() {
        int16_t handle = 0;
        acquire(handle);

        return at(handle);
    }

    void release(int16_t handle) {
        if((handle <= 0) || (handle >= m_poolSize))
            throw stException("stPool::acquire");

        m_free[m_freeSize++] = handle;
    }

    void release(Struct& pointer) {
        int16_t handle = &pointer - &m_pool[0];
        release(handle);
    }

    Struct const& at(int16_t handle) const {
        assert((handle >= 0) && (handle < m_poolSize));
        return m_pool[handle];
    }

    Struct& at(int16_t handle) {
        assert((handle >= 0) && (handle < m_poolSize));
        return m_pool[handle];
    }


private:
    int16_t const m_capacity;
    int16_t m_poolSize;
    int16_t m_freeSize;
    std::unique_ptr<Struct[]> m_pool;
    std::unique_ptr<int16_t[]> m_free;
};


#endif // SUBTILE_SUBCORE_H
