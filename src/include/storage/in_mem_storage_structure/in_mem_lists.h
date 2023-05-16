#pragma once

#include "storage/storage_structure/in_mem_file.h"
#include "storage/storage_structure/lists/list_headers.h"
#include "storage/storage_structure/lists/lists_metadata.h"

namespace kuzu {
namespace storage {

typedef std::vector<std::atomic<uint64_t>> atomic_uint64_vec_t;

class InMemLists;
class AdjLists;

using fill_in_mem_lists_function_t =
    std::function<void(InMemLists* inMemLists, uint8_t* defaultVal, PageByteCursor& pageByteCursor,
        common::offset_t nodeOffset, uint64_t posInList, const common::LogicalType& dataType)>;

class InMemListsUtils {
public:
    static inline void incrementListSize(
        atomic_uint64_vec_t& listSizes, uint32_t offset, uint32_t val) {
        assert(offset < listSizes.size());
        listSizes[offset].fetch_add(val, std::memory_order_relaxed);
    }
    static inline uint64_t decrementListSize(
        atomic_uint64_vec_t& listSizes, uint32_t offset, uint32_t val) {
        assert(offset < listSizes.size());
        return listSizes[offset].fetch_sub(val, std::memory_order_relaxed);
    }
};

class InMemLists {
public:
    InMemLists(std::string fName, common::LogicalType dataType, uint64_t numBytesForElement,
        uint64_t numNodes, std::shared_ptr<ListHeadersBuilder> listHeadersBuilder)
        : InMemLists{std::move(fName), std::move(dataType), numBytesForElement, numNodes} {
        this->listHeadersBuilder = std::move(listHeadersBuilder);
    }
    virtual ~InMemLists() = default;

    virtual void saveToFile();
    virtual void setElement(common::offset_t nodeOffset, uint64_t pos, uint8_t* val);
    virtual inline InMemOverflowFile* getInMemOverflowFile() { return nullptr; }
    inline ListsMetadataBuilder* getListsMetadataBuilder() { return listsMetadataBuilder.get(); }
    inline uint8_t* getMemPtrToLoc(uint64_t pageIdx, uint16_t posInPage) const {
        return inMemFile->getPage(pageIdx)->data + (posInPage * numBytesForElement);
    }

    void fillWithDefaultVal(uint8_t* defaultVal, uint64_t numNodes, ListHeaders* listHeaders);
    void initListsMetadataAndAllocatePages(
        uint64_t numNodes, ListHeaders* listHeaders, ListsMetadata* listsMetadata);

    // Calculates the page id and offset in page where the data of a particular list has to be put
    // in the in-mem pages.
    PageElementCursor calcPageElementCursor(uint64_t reversePos, uint8_t numBytesPerElement,
        common::offset_t nodeOffset, bool hasNULLBytes);

protected:
    InMemLists(std::string fName, common::LogicalType dataType, uint64_t numBytesForElement,
        uint64_t numNodes);

private:
    static void calculatePagesForList(uint64_t& numPages, uint64_t& offsetInPage,
        uint64_t numElementsInList, uint64_t numElementsPerPage);

    static inline void fillInMemListsWithNonOverflowValFunc(InMemLists* inMemLists,
        uint8_t* defaultVal, PageByteCursor& pageByteCursor, common::offset_t nodeOffset,
        uint64_t posInList, const common::LogicalType& dataType) {
        inMemLists->setElement(nodeOffset, posInList, defaultVal);
    }
    static void fillInMemListsWithStrValFunc(InMemLists* inMemLists, uint8_t* defaultVal,
        PageByteCursor& pageByteCursor, common::offset_t nodeOffset, uint64_t posInList,
        const common::LogicalType& dataType);
    static void fillInMemListsWithListValFunc(InMemLists* inMemLists, uint8_t* defaultVal,
        PageByteCursor& pageByteCursor, common::offset_t nodeOffset, uint64_t posInList,
        const common::LogicalType& dataType);
    static fill_in_mem_lists_function_t getFillInMemListsFunc(const common::LogicalType& dataType);

public:
    std::unique_ptr<InMemFile> inMemFile;

protected:
    std::string fName;
    common::LogicalType dataType;
    uint64_t numBytesForElement;
    std::unique_ptr<ListsMetadataBuilder> listsMetadataBuilder;
    std::shared_ptr<ListHeadersBuilder> listHeadersBuilder;
};

class InMemRelIDLists : public InMemLists {
public:
    InMemRelIDLists(std::string fName, uint64_t numNodes,
        std::shared_ptr<ListHeadersBuilder> listHeadersBuilder)
        : InMemLists{std::move(fName), common::LogicalType{common::LogicalTypeID::INTERNAL_ID},
              sizeof(common::offset_t), numNodes, std::move(listHeadersBuilder)} {}
};

class InMemListsWithOverflow : public InMemLists {
protected:
    InMemListsWithOverflow(std::string fName, common::LogicalType dataType, uint64_t numNodes,
        std::shared_ptr<ListHeadersBuilder> listHeadersBuilder);

    InMemOverflowFile* getInMemOverflowFile() override { return overflowInMemFile.get(); }
    void saveToFile() override;

protected:
    std::unique_ptr<InMemOverflowFile> overflowInMemFile;
};

class InMemAdjLists : public InMemLists {
public:
    InMemAdjLists(std::string fName, uint64_t numNodes)
        : InMemLists{std::move(fName), common::LogicalType(common::LogicalTypeID::INTERNAL_ID),
              sizeof(common::offset_t), numNodes} {
        listHeadersBuilder = make_shared<ListHeadersBuilder>(this->fName, numNodes);
    };

    void setElement(common::offset_t nodeOffset, uint64_t pos, uint8_t* val) override;

    void saveToFile() override;

    inline std::shared_ptr<ListHeadersBuilder> getListHeadersBuilder() const {
        return listHeadersBuilder;
    }
    inline uint32_t getListSize(common::offset_t nodeOffset) const {
        return listHeadersBuilder->getListSize(nodeOffset);
    }
};

class InMemStringLists : public InMemListsWithOverflow {
public:
    InMemStringLists(std::string fName, uint64_t numNodes,
        std::shared_ptr<ListHeadersBuilder> listHeadersBuilder)
        : InMemListsWithOverflow{std::move(fName),
              common::LogicalType(common::LogicalTypeID::STRING), numNodes,
              std::move(listHeadersBuilder)} {};
};

class InMemListLists : public InMemListsWithOverflow {
public:
    InMemListLists(std::string fName, common::LogicalType dataType, uint64_t numNodes,
        std::shared_ptr<ListHeadersBuilder> listHeadersBuilder)
        : InMemListsWithOverflow{
              std::move(fName), std::move(dataType), numNodes, std::move(listHeadersBuilder)} {};
};

class InMemListsFactory {
public:
    static std::unique_ptr<InMemLists> getInMemPropertyLists(const std::string& fName,
        const common::LogicalType& dataType, uint64_t numNodes,
        std::shared_ptr<ListHeadersBuilder> listHeadersBuilder = nullptr);
};

} // namespace storage
} // namespace kuzu
