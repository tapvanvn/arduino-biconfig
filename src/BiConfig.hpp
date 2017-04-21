///BiConfig v1.0
///author: Duy Nguyen <tapvanvn@gmail.com>
/// A format for configuration on weak device

#ifndef _H_BICONFIG
#define _H_BICONFIG

//#include <arduino.h>
#include <stdint.h>
#include <SimpleLinker.hpp>
#include <SD.h>

using namespace std;

class BiConfig
{
public:

    class IValue
    {
    public:
        virtual void write(File*) = 0;
        virtual uint8_t* cloneData() = 0;
    };

    template <typename T, uint8_t Header>
    class TValue : public IValue
    {
    public:
        TValue(T length, uint8_t* data)
        {
            _length = length;
            _data = data;
        }
        ~TValue()
        {
            if(_data)
            {
                delete[] _data;
                _data = 0;
                _length = 0;
            }
        }
        void write(File* file)
        {
            file->write((uint8_t)0x02);
            file->write((uint8_t)Header);
            file->write((uint8_t*)&_length,sizeof(T));
            if(_length > 0)
            {
                file->write(_data, _length);
            }
        }
        uint8_t* cloneData()
        {
            if(_length > 0)
            {
                uint8_t* clone_data = new uint8_t[_length];
                memcpy(clone_data, _data, _length);
                return clone_data;
            }
            return 0;
        }
    protected:
        T _length;
        uint8_t* _data;
    };

    typedef TValue<uint8_t, 0x21> I8Value;
    typedef TValue<uint16_t, 0x22> I16Value;
    typedef TValue<uint32_t, 0x23> I32Value;
    typedef TValue<uint64_t, 0x24> I64Value;

    class KeyValuePair
    {
    public:
        char* _key;
        IValue* _value;
        KeyValuePair() : _key(0), _value(0)
        {

        }
        ~KeyValuePair()
        {
            if(_key)
            {
                delete[] _key;
                _key = 0;
            }
            if(_value)
            {
                delete _value;
            }
        }
    };

    class Ledger : public SimpleLinker<KeyValuePair>
    {
    public:
        Ledger()
        {
            _begin = _end = 0;
        }

        ~Ledger()
        {
            //free memory
            while(_begin != 0)
            {
                Element* ptr = _begin;
                _begin = ptr->_next;

                if(ptr->_value)
                {
                    delete ptr->_value;
                }
                delete ptr;
            }
        }

        void loop( void(*callback)(Element*) )
        {
            Element* pointer = _begin;
            while(pointer != 0)
            {
                callback(pointer);
                pointer = pointer->_next;
            }
        }

        void add(Element* element)
        {
            if(!_begin) _begin = element;

            if(!_end)
            {
                _end = element;
            }
            else
            {
                _end->_next = element;
                _end = element;
            }
        }

        void add(const char* key, IValue* val)
        {
            Element* element = new Element();
            if(key)
            {
                char* element_key = new char[strlen(key) + 1];
                element_key[strlen(key)] = '\0';
                memcpy(element_key, key, strlen(key));

                element->_value = new KeyValuePair();
                element->_value->_key = element_key;
                element->_value->_value = val;
            }
            add(element);
        }



        void add(const char* key, uint8_t val)
        {
            I8Value* element_value = new I8Value(1, &val);
            add(key, element_value);
        }

        void add(const char* key, uint16_t val)
        {
            I8Value* element_value = new I8Value(2, (uint8_t*)&val);
            add(key, element_value);
        }

        void add(const char* key, uint32_t val)
        {
            I8Value* element_value = new I8Value(4, (uint8_t*)&val);
            add(key, element_value);
        }

        void add(const char* key, uint64_t val)
        {
            I8Value* element_value = new I8Value(8, (uint8_t*)&val);
            add(key, element_value);
        }

        void add(const char* key, uint8_t* data, uint8_t len)
        {
            I8Value* element_value = new I8Value(len,data);
            add(key, element_value);
        }

        void add(const char* key, uint8_t* data, uint16_t len)
        {
            I16Value* element_value = new I16Value(len,data);
            add(key, element_value);
        }

        void add(const char* key, uint8_t data, uint32_t len)
        {
            I32Value* element_value = new I32Value(len,data);
            add(key, element_value);
        }

        void add(const char* key, uint8_t data, uint64_t len)
        {
            I64Value* element_value = new I64Value(len,data);
            add(key, element_value);
        }

        void add(const char* key, const char* value)
        {
            uint64_t len = strlen(value);

            if(len >= UINT32_MAX)
            {
                I64Value* element_value = new I64Value((uint64_t)len + 1,(uint8_t*)value);
                add(key, element_value);
            }
            else if(len >= UINT16_MAX)
            {
                I32Value* element_value = new I32Value((uint32_t)len + 1,(uint8_t*)value);
                add(key, element_value);
            }
            else if(len >= UINT8_MAX)
            {
                I16Value* element_value = new I16Value((uint16_t)len + 1,(uint8_t*)value);
                add(key, element_value);
            }
            else
            {
                I8Value* element_value = new I8Value((uint8_t)len + 1,(uint8_t*)value);
                add(key, element_value);
            }
        }

        uint8_t* get(const char* path)
        {
            //Serial.print("get path:");
            //Serial.println(path);

            uint8_t key_level = 0;
            uint8_t group_level = 0;

            char *key_ptr = path, *key_last_ptr = path;

            Element* pointer = _begin;

            while(*key_ptr)
            {
                //Serial.println("loop");
                //find current key. the current key is determine by key_last_ptr and key_ptr
                while(*key_ptr && *key_ptr != '/')
                {
                    //Serial.println(*key_ptr, HEX);
                    key_ptr++;
                }

                //Serial.print("curretn:");
                //Serial.println(*key_ptr, DEC);
                //if not same level of group so move to the correct level
                while(key_level != group_level && pointer)
                {
                    if(!pointer->_value)
                    {
                        group_level--;
                    }
                    else if(!pointer->_value->_value)
                    {
                        group_level ++;
                        //Serial.print("group_level");
                        //Serial.println(group_level);
                    }
                    pointer = pointer->_next;
                }
                //find element match that key at same level
                while(pointer)
                {
                    if(pointer->_value)
                    {
                        char* element_key = pointer->_value->_key;

                        //Serial.print("element:");
                        //Serial.println(element_key);

                        //Serial.print("last:");
                        //Serial.println(key_last_ptr);

                        char* tmp_key_ptr = key_last_ptr;

                        //go over the matched characters
                        while(*element_key && *element_key++ == *tmp_key_ptr++)
                        {
                            ;
                        }

                        if(!*element_key && tmp_key_ptr == key_ptr)
                        {
                            //Serial.println("found match element");
                            //found element if no sub group request so return value
                            if(!*key_ptr)
                            {
                                //Serial.println("end search");
                                if (pointer->_value->_value)
                                {
                                    return pointer->_value->_value->cloneData();
                                }
                                return 0;
                            }
                            break;
                        }
                        else
                        {
                            //if we met a subgroup but not match the key so pass it
                            if(!pointer->_value->_value)
                            {
                                int count_level = 1;
                                while(pointer->_value && count_level > 0)
                                {
                                    if(!pointer->_value)
                                    {
                                        count_level--;
                                    }
                                    if(pointer->_value && pointer->_value->_key && !pointer->_value->_value)
                                    {
                                        count_level ++;
                                    }

                                    pointer = pointer->_next;
                                }
                            }
                        }
                    }
                    pointer = pointer->_next;
                }

                //key is / so go next character
                //key_ptr++;
                key_level++;
                key_last_ptr = ++key_ptr;
                //Serial.print("key level:");
                //Serial.println(key_level);
            }

            return 0;
        }

        void addGroup(const char* key, Ledger* sub_ledger )
        {
            add(key, (IValue*)0);

            if(sub_ledger && sub_ledger->_begin)
            {
                if(!_begin)
                {
                    _begin = sub_ledger->_begin;
                }
                else
                {
                    _end->_next = sub_ledger->_begin;
                }
                _end = sub_ledger->_end;

                sub_ledger->_begin = sub_ledger->_end = 0;
            }

            add((const char*) 0, (IValue*) 0);

        }

        void write( File* file)
        {
            Element* pointer = _begin;
            while(pointer != 0)
            {
                if(pointer->_value)
                {
                    file->write((uint8_t)0x01);
                    file->write(pointer->_value->_key, strlen(pointer->_value->_key) + 1);

                    if(pointer->_value->_value)
                    {
                        pointer->_value->_value->write(file);
                    }
                }
                else
                {
                    file->write((uint8_t)0x10);
                }

                pointer = pointer->_next;
            }
        }
    private:
        Element *_begin, *_end;
    };


    static Ledger* read(File* file)
    {
        //Serial.println("begin read");
        Ledger* ledger = new Ledger();

        uint8_t last_sign = 0x00;
        char* curr_key = 0;
        while(file->available())
        {
            uint8_t sign = file->read();

            if(sign == 0x01)
            {
                //read key
                char* key = readKey(file);
                //Serial.print("key:");
                //Serial.println(key);

                if(last_sign == 0x01)
                {
                    //if last sign is a key so we begin a sub group

                    Ledger::Element* element = new Ledger::Element();
                    element->_value = new KeyValuePair();
                    element->_value->_key = curr_key;
                    ledger->add(element);
                }

                curr_key = key; //we dont need to delete last key here becouse it had beed used at other place.

            }
            else if(sign == 0x02)
            {
                //Serial.println("pair");
                //read value
                Ledger::Element* element = new Ledger::Element();
                element->_value = new KeyValuePair();

                //Serial.print("Key:");
                //Serial.println(curr_key);

                element->_value->_key = curr_key;
                element->_value->_value = readValue(file);
                ledger->add(element);
            }
            else if(sign == 0x10)
            {
                //end group
                Ledger::Element* element = new Ledger::Element();
                ledger->add(element);
            }
            last_sign = sign;
        }
        return ledger;
    }

private:
    static char* readKey(File* file)
    {
        String str = "";
        char chr = file->read();
        while(chr)
        {
            str += chr;
            chr = file->read();
        }

        char* chr_str = new char[str.length() + 1];
        memcpy(chr_str, str.c_str(), str.length());
        chr_str[str.length()] = '\0';
        return chr_str;
    }
    static IValue* readValue(File* file)
    {
        //Serial.println("readValue");
        uint8_t type = file->read();

        if(type == 0x21)
        {
            uint8_t len = file->read();
            uint8_t* data = new uint8_t[len];
            file->read(data, len);
            I8Value* val = new I8Value(len, data);
            return val;
        }
        else if(type == 0x22)
        {
            uint16_t len = file->read() << 8 | file->read();
            uint8_t* data = new uint8_t[len];
            file->read(data, len);
            I16Value* val = new I16Value(len, data);
            return val;
        }
        else if(type == 0x23)
        {
            uint8_t buff[4];
            for(uint8_t i = 0; i< 4; i++)
            {
                buff[i] = file->read();
            }
            uint32_t len = *(uint32_t*) buff;
            uint8_t* data = new uint8_t[len];
            file->read(data, len);
            I32Value* val = new I32Value(len, data);
            return val;
        }
        else if(type == 0x24)
        {
            uint8_t buff[8];
            for(uint8_t i = 0; i< 8; i++)
            {
                buff[i] = file->read();
            }
            uint64_t len = *(uint64_t*) buff;
            uint8_t* data = new uint8_t[len];
            file->read(data, len);
            I64Value* val = new I64Value(len, data);
            return val;
        }
        return 0;
    }
};

#endif //_H_BICONFIG
