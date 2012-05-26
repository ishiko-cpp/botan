/*
* TLS Record Handling
* (C) 2004-2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_RECORDS_H__
#define BOTAN_TLS_RECORDS_H__

#include <botan/tls_ciphersuite.h>
#include <botan/tls_alert.h>
#include <botan/tls_magic.h>
#include <botan/tls_version.h>
#include <botan/pipe.h>
#include <botan/mac.h>
#include <vector>
#include <functional>

namespace Botan {

namespace TLS {

class Session_Keys;

/**
* TLS Record Writer
*/
class BOTAN_DLL Record_Writer
   {
   public:
      void send(byte type, const byte input[], size_t length);
      void send(byte type, byte val) { send(type, &val, 1); }

      void send(byte type, const std::vector<byte>& input)
         { send(type, &input[0], input.size()); }

      std::vector<byte> send(class Handshake_Message& msg);

      void send_alert(const Alert& alert);

      void activate(Connection_Side side,
                    const Ciphersuite& suite,
                    const Session_Keys& keys,
                    byte compression_method);

      void set_version(Protocol_Version version);

      void reset();

      void set_maximum_fragment_size(size_t max_fragment);

      Record_Writer(std::function<void (const byte[], size_t)> output_fn);

      ~Record_Writer() { delete m_mac; }
   private:
      Record_Writer(const Record_Writer&) {}
      Record_Writer& operator=(const Record_Writer&) { return (*this); }

      void send_record(byte type, const byte input[], size_t length);

      std::function<void (const byte[], size_t)> m_output_fn;

      std::vector<byte> m_writebuf;

      Pipe m_cipher;
      MessageAuthenticationCode* m_mac;

      size_t m_block_size, m_mac_size, m_iv_size, m_max_fragment;

      u64bit m_seq_no;
      Protocol_Version m_version;
   };

/**
* TLS Record Reader
*/
class BOTAN_DLL Record_Reader
   {
   public:

      /**
      * @param input new input data (may be NULL if input_size == 0)
      * @param input_size size of input in bytes
      * @param input_consumed is set to the number of bytes of input
      *        that were consumed
      * @param msg_type is set to the type of the message just read if
      *        this function returns 0
      * @param msg is set to the contents of the record
      * @return number of bytes still needed (minimum), or 0 if success
      */
      size_t add_input(const byte input[], size_t input_size,
                       size_t& input_consumed,
                       byte& msg_type,
                       std::vector<byte>& msg);

      void activate(Connection_Side side,
                    const Ciphersuite& suite,
                    const Session_Keys& keys,
                    byte compression_method);

      void set_version(Protocol_Version version);

      void reset();

      void set_maximum_fragment_size(size_t max_fragment);

      Record_Reader();

      ~Record_Reader() { delete m_mac; }
   private:
      Record_Reader(const Record_Reader&) {}
      Record_Reader& operator=(const Record_Reader&) { return (*this); }

      size_t fill_buffer_to(const byte*& input,
                            size_t& input_size,
                            size_t& input_consumed,
                            size_t desired);

      std::vector<byte> m_readbuf;
      std::vector<byte> m_macbuf;
      size_t m_readbuf_pos;

      Pipe m_cipher;
      MessageAuthenticationCode* m_mac;
      size_t m_block_size, m_iv_size, m_max_fragment;
      u64bit m_seq_no;
      Protocol_Version m_version;
   };

}

}

#endif
