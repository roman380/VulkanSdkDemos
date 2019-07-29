#include <assert.h>
#include <iostream>

#include <dxgi1_6.h>
#include <d3d11_4.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#if defined(_DEBUG)
	#define verify assert
#else
	#define verify(a) (a)
#endif
#define VERIFY verify

int main()
{
	#pragma region Initialization
    vk::Instance inst;
	{
		auto const app = vk::ApplicationInfo()
							 .setPApplicationName("BindImageMemory2")
							 .setApplicationVersion(0)
							 .setPEngineName("BindImageMemory2")
							 .setEngineVersion(0)
							 .setApiVersion(VK_API_VERSION_1_0);

		const char* enabled_layers[1] = { "VK_LAYER_KHRONOS_validation" };
		size_t enabled_layer_count = _countof(enabled_layers);

		const char* enabled_extension_names[16];
		size_t enabled_extension_count = 0;
		enabled_extension_names[enabled_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
		enabled_extension_names[enabled_extension_count++] = VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME;

		auto const inst_info = vk::InstanceCreateInfo()
								   .setPApplicationInfo(&app)
								   .setEnabledLayerCount(enabled_layer_count)
								   .setPpEnabledLayerNames(enabled_layers)
								   .setEnabledExtensionCount(enabled_extension_count)
								   .setPpEnabledExtensionNames(enabled_extension_names);
		VERIFY(vk::createInstance(&inst_info, nullptr, &inst) == vk::Result::eSuccess);
	}
    vk::PhysicalDevice gpu;
	{
		uint32_t gpu_count;
		VERIFY(inst.enumeratePhysicalDevices(&gpu_count, static_cast<vk::PhysicalDevice *>(nullptr)) == vk::Result::eSuccess);
		assert(gpu_count);
		std::unique_ptr<vk::PhysicalDevice[]> physical_devices(new vk::PhysicalDevice[gpu_count]);
		VERIFY(inst.enumeratePhysicalDevices(&gpu_count, physical_devices.get()) == vk::Result::eSuccess);
		gpu = physical_devices[0];
	}
    vk::Device device;
	static const uint32_t graphics_queue_family_index = 0;
	vk::Queue graphics_queue;
	{
	    float const priorities[1] = {0.0};

		const char* enabled_extension_names[16];
		size_t enabled_extension_count = 0;
		enabled_extension_names[enabled_extension_count++] = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
		enabled_extension_names[enabled_extension_count++] = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
		enabled_extension_names[enabled_extension_count++] = VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME;
		enabled_extension_names[enabled_extension_count++] = VK_KHR_BIND_MEMORY_2_EXTENSION_NAME;

	    uint32_t queue_family_count;
		gpu.getQueueFamilyProperties(&queue_family_count, static_cast<vk::QueueFamilyProperties *>(nullptr));
		assert(queue_family_count >= 1);
	    std::unique_ptr<vk::QueueFamilyProperties[]> queue_props;
		queue_props.reset(new vk::QueueFamilyProperties[queue_family_count]);
		gpu.getQueueFamilyProperties(&queue_family_count, queue_props.get());

		vk::DeviceQueueCreateInfo queues[2];
		queues[0].setQueueFamilyIndex(graphics_queue_family_index);
		queues[0].setQueueCount(1);
		queues[0].setPQueuePriorities(priorities);

		auto deviceInfo = vk::DeviceCreateInfo()
							  .setQueueCreateInfoCount(1)
							  .setPQueueCreateInfos(queues)
							  .setEnabledLayerCount(0)
							  .setPpEnabledLayerNames(nullptr)
							  .setEnabledExtensionCount(enabled_extension_count)
							  .setPpEnabledExtensionNames(enabled_extension_names)
							  .setPEnabledFeatures(nullptr);

		VERIFY(gpu.createDevice(&deviceInfo, nullptr, &device) == vk::Result::eSuccess);
	    device.getQueue(graphics_queue_family_index, 0, &graphics_queue);
	}
    vk::CommandPool cmd_pool;
	//vk::CommandBuffer cmd;
	{
		auto const cmd_pool_info = vk::CommandPoolCreateInfo().setQueueFamilyIndex(graphics_queue_family_index);
		VERIFY(device.createCommandPool(&cmd_pool_info, nullptr, &cmd_pool) == vk::Result::eSuccess);

		auto const cmd = vk::CommandBufferAllocateInfo()
							 .setCommandPool(cmd_pool)
							 .setLevel(vk::CommandBufferLevel::ePrimary)
							 .setCommandBufferCount(1);

		//VERIFY(device.allocateCommandBuffers(&cmd, &cmd) == vk::Result::eSuccess);
	}

	#pragma endregion

	VkPhysicalDeviceExternalImageFormatInfo PhysicalDeviceExternalImageFormatInfo = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO };
	PhysicalDeviceExternalImageFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
	VkPhysicalDeviceImageFormatInfo2 PhysicalDeviceImageFormatInfo2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2 };
	PhysicalDeviceImageFormatInfo2.pNext = &PhysicalDeviceExternalImageFormatInfo;
	PhysicalDeviceImageFormatInfo2.format = VK_FORMAT_R8G8B8A8_UNORM;
	PhysicalDeviceImageFormatInfo2.type = VK_IMAGE_TYPE_2D;
	PhysicalDeviceImageFormatInfo2.tiling = VK_IMAGE_TILING_OPTIMAL;
	PhysicalDeviceImageFormatInfo2.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	VkExternalImageFormatProperties ExternalImageFormatProperties = { VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES };
	VkImageFormatProperties2 ImageFormatProperties2 = { VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2 };
	ImageFormatProperties2.pNext = &ExternalImageFormatProperties;
	verify(vkGetPhysicalDeviceImageFormatProperties2(gpu, &PhysicalDeviceImageFormatInfo2, &ImageFormatProperties2) == VK_SUCCESS);
	assert(ExternalImageFormatProperties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
	assert(ExternalImageFormatProperties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
	assert(ExternalImageFormatProperties.externalMemoryProperties.compatibleHandleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT);

	VkExtent3D Extent = { 1920, 1080, 1 };

	VkExternalMemoryImageCreateInfo ExternalMemoryImageCreateInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
	ExternalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
	VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	ImageCreateInfo.pNext = &ExternalMemoryImageCreateInfo;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	ImageCreateInfo.extent = Extent;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImage Image;
	VERIFY(vkCreateImage(device, &ImageCreateInfo, nullptr, &Image) == VK_SUCCESS);

	VkMemoryDedicatedRequirements MemoryDedicatedRequirements = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };
	VkMemoryRequirements2 MemoryRequirements2 = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
	MemoryRequirements2.pNext = &MemoryDedicatedRequirements;
	VkImageMemoryRequirementsInfo2 ImageMemoryRequirementsInfo2 = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2 };
	ImageMemoryRequirementsInfo2.image = Image;
	// WARN: Memory access violation unless validation instance layer is enabled, otherwise success but...
	vkGetImageMemoryRequirements2(device, &ImageMemoryRequirementsInfo2, &MemoryRequirements2);
	//       ... if we happen to be here, MemoryRequirements2 is empty
	VkMemoryRequirements& MemoryRequirements = MemoryRequirements2.memoryRequirements;
		
	assert(!MemoryRequirements.size);
	vkGetImageMemoryRequirements(device, Image, &MemoryRequirements);
	assert(MemoryRequirements.size);

	IDXGIFactory* DxgiFactory;
	VERIFY(SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&DxgiFactory))));
	IDXGIAdapter* DxgiAdapter;
	VERIFY(SUCCEEDED(DxgiFactory->EnumAdapters(0, &DxgiAdapter)));
	DXGI_ADAPTER_DESC DxgiAdapterDesc;
	VERIFY(SUCCEEDED(DxgiAdapter->GetDesc(&DxgiAdapterDesc)));

	ID3D11Device* D3d11Device;
	D3D_FEATURE_LEVEL FeatureLevel;
	ID3D11DeviceContext* D3d11DeviceContext;
	VERIFY(SUCCEEDED(D3D11CreateDevice(DxgiAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &D3d11Device, &FeatureLevel, &D3d11DeviceContext)));
	CD3D11_TEXTURE2D_DESC TextureDesc(DXGI_FORMAT_R8G8B8A8_UNORM, ImageCreateInfo.extent.width, ImageCreateInfo.extent.height, 1, 1);
	TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
	ID3D11Texture2D* Texture;
	VERIFY(SUCCEEDED(D3d11Device->CreateTexture2D(&TextureDesc, nullptr, &Texture)));
	IDXGIResource1* DxgiResource1;
	VERIFY(SUCCEEDED(Texture->QueryInterface(&DxgiResource1)));
	HANDLE Handle;
	VERIFY(SUCCEEDED(DxgiResource1->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &Handle)));

	VkMemoryDedicatedAllocateInfo MemoryDedicatedAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO };
	MemoryDedicatedAllocateInfo.image = Image;
	VkImportMemoryWin32HandleInfoKHR ImportMemoryWin32HandleInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR };
	ImportMemoryWin32HandleInfo.pNext = &MemoryDedicatedAllocateInfo;
	ImportMemoryWin32HandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
	ImportMemoryWin32HandleInfo.handle = Handle;
	VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	MemoryAllocateInfo.pNext = &ImportMemoryWin32HandleInfo;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	// WARN: MemoryAllocateInfo.memoryTypeIndex remains zero
	VkDeviceMemory ImageMemory;
	VERIFY(vkAllocateMemory(device, &MemoryAllocateInfo, nullptr, &ImageMemory) == VK_SUCCESS);
	VkBindImageMemoryInfo BindImageMemoryInfo = { VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO };
	BindImageMemoryInfo.image = Image;
	BindImageMemoryInfo.memory = ImageMemory;
	VERIFY(vkBindImageMemory2(device, 1, &BindImageMemoryInfo) == VK_SUCCESS);

	VkImage SourceImage;
	VkDeviceMemory SourceImageMemory;
	{
		VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		ImageCreateInfo.extent = Extent;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VERIFY(vkCreateImage(device, &ImageCreateInfo, nullptr, &SourceImage) == VK_SUCCESS);

		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements(device, SourceImage, &MemoryRequirements);
		VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		VERIFY(vkAllocateMemory(device, &MemoryAllocateInfo, nullptr, &SourceImageMemory) == VK_SUCCESS);
		VERIFY(vkBindImageMemory(device, SourceImage, SourceImageMemory, 0) == VK_SUCCESS);
	}

	{
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		CommandBufferAllocateInfo.commandPool = cmd_pool;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		VkCommandBuffer CommandBuffer;
		VERIFY(vkAllocateCommandBuffers(device, &CommandBufferAllocateInfo, &CommandBuffer) == VK_SUCCESS);

		VkCommandBufferBeginInfo CommandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		VERIFY(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo) == VK_SUCCESS);

		{
			VkImageMemoryBarrier ImageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			ImageMemoryBarrier.srcAccessMask = 0;
			ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			ImageMemoryBarrier.image = SourceImage;
			ImageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
		}
		{
			VkImageMemoryBarrier ImageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			ImageMemoryBarrier.srcAccessMask = 0;
			ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			ImageMemoryBarrier.image = Image;
			ImageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
		}
		VkImageCopy ImageCopy = { };
		ImageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageCopy.srcSubresource.layerCount = 1;
		ImageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageCopy.dstSubresource.layerCount = 1;
		ImageCopy.extent = Extent;
		vkCmdCopyImage(CommandBuffer, SourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ImageCopy);
		{
			VkImageMemoryBarrier ImageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			ImageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			ImageMemoryBarrier.image = Image;
			ImageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
		}

		VERIFY(vkEndCommandBuffer(CommandBuffer) == VK_SUCCESS);

		VkFenceCreateInfo FenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		VkFence Fence;
		VERIFY(vkCreateFence(device, &FenceCreateInfo, nullptr, &Fence) == VK_SUCCESS);
		VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;
		VERIFY(vkQueueSubmit(graphics_queue, 1, &SubmitInfo, Fence) == VK_SUCCESS);

		VERIFY(vkWaitForFences(device, 1, &Fence, VK_FALSE, UINT64_MAX) == VK_SUCCESS);
	}
}
