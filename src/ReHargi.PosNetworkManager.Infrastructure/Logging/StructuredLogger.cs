using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace ReHargi.PosNetworkManager.Infrastructure.Logging
{
    /// <summary>
    /// Structured logging service for application operations.
    /// Logs are stored in ProgramData directory.
    /// </summary>
    public class StructuredLogger
    {
        private readonly string _logDirectory;
        private readonly object _lockObj = new object();

        public StructuredLogger()
        {
            _logDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData),
                Core.Constants.AppConstants.StorageRoot,
                Core.Constants.AppConstants.LogsSubfolder);

            EnsureDirectoryExists();
        }

        private void EnsureDirectoryExists()
        {
            if (!Directory.Exists(_logDirectory))
                Directory.CreateDirectory(_logDirectory);
        }

        public void LogInformation(string operation, string target, string message,
            string adapter = null, string ipAddress = null, string macAddress = null)
        {
            LogEntry(LogLevel.Information, operation, target, message, adapter, ipAddress, macAddress);
        }

        public void LogWarning(string operation, string target, string message,
            string adapter = null, string ipAddress = null, string macAddress = null)
        {
            LogEntry(LogLevel.Warning, operation, target, message, adapter, ipAddress, macAddress);
        }

        public void LogError(string operation, string target, string message, int? errorCode = null,
            string adapter = null, string ipAddress = null, string macAddress = null)
        {
            LogEntry(LogLevel.Error, operation, target, message, adapter, ipAddress, macAddress, errorCode);
        }

        private void LogEntry(LogLevel level, string operation, string target, string message,
            string adapter, string ipAddress, string macAddress, int? errorCode = null)
        {
            lock (_lockObj)
            {
                try
                {
                    string logFile = Path.Combine(_logDirectory, 
                        $"ReHargi-POS-{DateTime.Now:yyyy-MM-dd}.log");

                    var sb = new StringBuilder();
                    sb.AppendFormat("[{0:yyyy-MM-dd HH:mm:ss.fff}] ", DateTime.Now);
                    sb.AppendFormat("[{0}] ", level);
                    sb.AppendFormat("[Operation: {0}] ", operation);
                    if (!string.IsNullOrEmpty(target))
                        sb.AppendFormat("[Target: {0}] ", target);
                    if (!string.IsNullOrEmpty(adapter))
                        sb.AppendFormat("[Adapter: {0}] ", adapter);
                    if (!string.IsNullOrEmpty(ipAddress))
                        sb.AppendFormat("[IP: {0}] ", ipAddress);
                    if (!string.IsNullOrEmpty(macAddress))
                        sb.AppendFormat("[MAC: {0}] ", macAddress);
                    if (errorCode.HasValue)
                        sb.AppendFormat("[ErrorCode: {0}] ", errorCode);
                    sb.AppendFormat("Message: {0}", message);

                    File.AppendAllText(logFile, sb.ToString() + Environment.NewLine);
                }
                catch
                {
                    // Silently fail if logging fails
                }
            }
        }
    }

    public enum LogLevel
    {
        Information,
        Warning,
        Error
    }
}
