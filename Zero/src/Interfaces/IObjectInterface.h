#pragma once


class IObjectInterface
{
public:
    // Code here for both OGL AND VULKAN
    virtual ~IObjectInterface() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void Delete() = 0;
};
