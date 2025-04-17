#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>   // 00_base_code
#include <iostream>  // 00_base_code
#include <stdexcept> // 00_base_code
#include <cstring>   // 02_validation_layers
#include <vector>    // 02_validation_layers
#include <optional>  // 03_physical_device_selection
#include <set>       // 05_window_surface

// window size
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation" // Khronos standard validation layer
};

#ifdef NDEBUG // no debug
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily; // for rendering instructions
    std::optional<uint32_t> presentFamily; // for swap chain

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};


class DoGlIce {
public:
    // init DoGlIce
    bool init() {
        if (!initWindow()) return false;
        if (!initVulkan()) return false;

        return true;
    }

    int run() {
        mainLoop();

        return 0;
    }
    
    ~DoGlIce() {
        cleanup();
    }

private:
    // local variables
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    // Abstracts the OS window surface in a platform-independent way (used for swapchain creation).
    VkSurfaceKHR surface; // KHR : Khronos extension

    // Select Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device; // logical device (physical device handler)

    // Queue Family
    VkQueue graphicsQueue; // for rendering instructions
    VkQueue presentQueue;  // for swap chain

    // called by init()
    bool initWindow() {
        if (!glfwInit()) {
            std::cout << "glfwInit() failed." << '\n';
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Doglice_v.0.0.1", nullptr, nullptr);
        if (!window) {
            std::cout << "glfwCreateWindow() failed." << '\n';
            return false;
        }

        return true;
    }

    // called by init()
    bool initVulkan() {
        if (!createInstance()) {
            std::cout << "createInstance() failed." << '\n';
            return false;
        };
        if (!setupDebugMessenger()) {
            std::cout << "setupDebugMessenger() failed." << '\n';
            return false;
        };
        if (!createSurface()) {
            std::cout << "createSurface() failed." << '\n';
            return false;
        }
        if (!pickPhysicalDevice()) {
            std::cout << "pickPhysicalDevice() failed." << '\n';
            return false;
        };
        if (!createLogicalDevice()) {
            std::cout << "createLogicalDevice() failed." << '\n';
            return false;
        };

        return true;
    }

    // called by initVulkan()
    bool createInstance() {
        // check Validation Layer
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            std::cout << "validation layers requested, but not available!" << '\n';
            return false;
        }

        VkApplicationInfo appInfo{};                           // set struct 0
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;    // set type as app_info
        appInfo.pApplicationName = "Doglice_v.0.0.1";          // our application name (could be changed)
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1); // app version
        appInfo.pEngineName = "DoGlIce Engine";                // our Engine name
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);      // engine version
        appInfo.apiVersion = VK_API_VERSION_1_0;               // Vulkan api version TODO: have to change api version

        VkInstanceCreateInfo createInfo{}; // set 0
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // set type as instance_info
        createInfo.pApplicationInfo = &appInfo;

        // extensions
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Validation Layer
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        // real creation;
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            std::cout << "failed to create instance!" << '\n';
            return false;
        }

        return true;
    }

    // called by createInstance()
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    // called by createInstance()
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }


    // called by initVulkan()
    bool setupDebugMessenger() {
        if (!enableValidationLayers) {
            return true;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            std::cout << "failed to set up debug messenger!" << '\n';
            return false;
        }

        return true;
    }

    // called by createInstance() and setupDebugMessenger()
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    // called by populateDebugMessengerCreateInfo()
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    
    // called by initVulkan()
    bool pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            std::cout << "failed to find GPUs with Vulkan support!" << '\n';
            return false;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            std::cout << "failed to find a suitable GPU!" << '\n';
            return false;
        }

        return true;
    }

    // called by pickPhysicalDevice()
    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }

    // called by isDeviceSuitable()
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }


    // called by initVulkan()
    bool createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = 0;

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            std::cout << "failed to create logical device!" << '\n';
            return false;
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        return true;
    }


    // called by initVulkan()
    bool createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            std::cout << "failed to create window surface!" << '\n';
            return false;
        }
        
        return true;
    }


    // called by run()
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }


    // called by DoGlIce Destructor
    void cleanup() {
        vkDestroyDevice(device, nullptr); // destroy logical device

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
};


int main() {
    DoGlIce app;

    // init application
    if (!app.init()) {
        std::cout << "Initialization failed." << '\n';
        return -1;
    }

    // run application
    return app.run();
}
