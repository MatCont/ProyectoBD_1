#ifndef _rbfm_h_
#define _rbfm_h_

#include <string>
#include <vector>

#include "../RecordBasedFile/PagedFileManager.h"

using namespace std;


// Id de Registro
typedef struct
{
    unsigned pageNum;
    unsigned slotNum;
} RID;


// Atributo
typedef enum { TypeInt = 0, TypeReal, TypeVarChar } AttrType;
typedef unsigned AttrLength;

struct Attribute {
    string   name;     // attribute name
    AttrType type;     // attribute type
    AttrLength length; // attribute length
};


/***
 * Directorio de slots que almacena offsets a los registros y sus tamaños
***/
struct PageDirSlot
{
    /**
     * offset (en bytes) al inicio de un registro desde el inicio de la página de datos
    **/
    unsigned int _offRecord;
    /**
     * tamaño del registro (en bytes)
    **/
    unsigned int _szRecord;
};


class RecordBasedFileManager
{
public:
    static RecordBasedFileManager* instance();

    RC createFile(const string &fileName);

    RC destroyFile(const string &fileName);

    RC openFile(const string &fileName, FileHandle &fileHandle);

    RC closeFile(FileHandle &fileHandle);

    /***
    * Formato de los datos pasados a la función:
    *  1) data es la concatenación de valores de atributos/campos
    *  2) Para int y float se usan 4 bytes para guardar su valor;
    *     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
    *  !!!The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute()
     ***/
    RC insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);

    RC readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);

    /***
     * Obtener página de datos (insertar nueva si es necesario) que tiene suficiente espacio libre para registro especificado
    ****/
    RC getDataPage(FileHandle &fileHandle, const unsigned int recordSize, PageNum& foundPage, PageNum& headerPage, unsigned int& freeSpaceLeftInPage);

    /***
     * Encontrar lugar para en nuevo registro en página especificada
    ***/
    RC findRecordSlot(FileHandle &fileHandle, PageNum pagenum, const unsigned int szRecord, PageDirSlot& slot, unsigned int& slotNum, unsigned int freeSpaceLeftInPage);

    RC printRecord(const vector<Attribute> &recordDescriptor, const void *data);

protected:
    RecordBasedFileManager();
    ~RecordBasedFileManager();

private:
    static RecordBasedFileManager *_rbf_manager;
    static PagedFileManager *_pfm;
};


// tamaño de TombStone
#define TOMBSTONE_SIZE (sizeof(RID))	//page number, slot number

// máximo de tamaño de un registro. El tamaño máximo que puede ser insertado en un registro + correspondiente metadata
#define MAX_SIZE_OF_RECORD (PAGE_SIZE - sizeof(unsigned int) - sizeof(unsigned int) - sizeof(PageDirSlot))

// declaración de tipos
typedef unsigned PageIdNum;
typedef unsigned PageId;

/***
 * Información sobre la página
***/
struct PageInfo
{
    // id de página (int) - índice de 0 a N-1, donde N es el número total de páginas en un archivo y 0th es el header
    PageId _pageid;
    // espacio libre en bytes
    unsigned int _numFreeBytes;
};

#define NUM_OF_PAGE_IDS ( PAGE_SIZE - sizeof(PageNum) - sizeof(PageIdNum) ) / sizeof(PageInfo)

/***
 * Header de página
***/
struct Header
{
    // id del siguiente header de página
    PageNum _nextHeaderPageId;

    // número de Ids de página usados en este header
    PageIdNum _numUsedPageIds;

    // arreglo de Ids de página, que ocupan el resto del header de archivo
    PageInfo _arrOfPageIds[ NUM_OF_PAGE_IDS ];
};

/***
 * Función para determinar el tamaño en bytes de un registro
***/
unsigned int sizeOfRecord(const vector<Attribute> &recordDescriptor, const void *data);

#endif
