#pragma once
#include <fstream>
#include <string>

/// @brief Abstract base class for loading configurations from files, to be extended by specific
/// loaders for different game components.
class Loader {
protected:
    /// @brief The filename to load from or save to.
    std::string filename;

private:
    /// @brief File buffer for reading/writing operations.
    std::filebuf fileBuffer;

public:
    /// @brief Abstract method to be implemented by derived classes to load specific configurations.
    virtual void loadConfig() = 0;

    Loader(std::string filename);
    virtual ~Loader();
};
