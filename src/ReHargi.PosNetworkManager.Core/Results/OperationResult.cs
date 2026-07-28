namespace ReHargi.PosNetworkManager.Core
{
    /// <summary>
    /// Result of an operation, with success status and optional error message.
    /// </summary>
    public class OperationResult
    {
        public bool IsSuccessful { get; set; }
        public string ErrorMessage { get; set; }
        public int? ErrorCode { get; set; }
        public string TechnicalDetails { get; set; }

        public static OperationResult Success()
        {
            return new OperationResult { IsSuccessful = true };
        }

        public static OperationResult Failure(string errorMessage, int? errorCode = null, string technicalDetails = null)
        {
            return new OperationResult
            {
                IsSuccessful = false,
                ErrorMessage = errorMessage,
                ErrorCode = errorCode,
                TechnicalDetails = technicalDetails
            };
        }
    }

    /// <summary>
    /// Generic result of an operation with a return value.
    /// </summary>
    public class OperationResult<T> : OperationResult
    {
        public T Data { get; set; }

        public static OperationResult<T> Success(T data)
        {
            return new OperationResult<T>
            {
                IsSuccessful = true,
                Data = data
            };
        }

        public static new OperationResult<T> Failure(string errorMessage, int? errorCode = null, string technicalDetails = null)
        {
            return new OperationResult<T>
            {
                IsSuccessful = false,
                ErrorMessage = errorMessage,
                ErrorCode = errorCode,
                TechnicalDetails = technicalDetails
            };
        }
    }
}
