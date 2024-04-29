
#ifndef CA_RESULT_H_
#define CA_RESULT_H_


// An enum containing CAudio error types
typedef enum CA_Result
{
  CA_SUCCESS = 0,   // No error

  CA_ERR_PA,        // PortAudio error
  CA_ERR_INVALID,   // Invalid operation
  CA_ERR_ALLOC,     // Memory allocation error
  CA_ERR_FILE,      // Error opening a file
  CA_ERR_FTYPE,     // A file had an incorrect/unsupported value
  CA_ERR_OUT,       // Error writing to a file
  CA_ERR_IN,        // Error reading from a file
}
CA_Result;


#endif // CA_RESULT_H_
