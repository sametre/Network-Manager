using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using ReHargi.PosNetworkManager.Core.Models;
using ReHargi.PosNetworkManager.Core.Constants;

namespace ReHargi.PosNetworkManager.Infrastructure.Storage
{
    /// <summary>
    /// Base backup/restore functionality for network configurations.
    /// </summary>
    public class BackupManager
    {
        private readonly string _backupDirectory;

        public BackupManager()
        {
            _backupDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData),
                AppConstants.StorageRoot,
                AppConstants.BackupsSubfolder);

            EnsureDirectoryExists();
        }

        private void EnsureDirectoryExists()
        {
            if (!Directory.Exists(_backupDirectory))
                Directory.CreateDirectory(_backupDirectory);
        }

        /// <summary>
        /// Create a backup of current configuration.
        /// </summary>
        public string BackupConfiguration(string adapterId, NetworkConfiguration config)
        {
            try
            {
                string filename = $"backup_{adapterId}_{DateTime.Now:yyyyMMdd_HHmmss}.json";
                string filepath = Path.Combine(_backupDirectory, filename);

                var backup = new ConfigurationBackup
                {
                    CreatedTime = DateTime.Now,
                    AdapterId = adapterId,
                    Configuration = config,
                    ComputerName = Environment.MachineName,
                    UserName = Environment.UserName
                };

                // TODO: Serialize to JSON
                // For now, stub
                return filepath;
            }
            catch
            {
                return null;
            }
        }

        /// <summary>
        /// List all available backups.
        /// </summary>
        public List<ConfigurationBackup> ListBackups()
        {
            var backups = new List<ConfigurationBackup>();
            try
            {
                foreach (var file in Directory.GetFiles(_backupDirectory, "backup_*.json"))
                {
                    // TODO: Deserialize from JSON
                }
            }
            catch
            {
                // Silently fail
            }

            return backups;
        }

        /// <summary>
        /// Delete a backup file.
        /// </summary>
        public bool DeleteBackup(string backupFilePath)
        {
            try
            {
                if (File.Exists(backupFilePath))
                {
                    File.Delete(backupFilePath);
                    return true;
                }
            }
            catch
            {
                // Silently fail
            }

            return false;
        }
    }

    /// <summary>
    /// Represents a configuration backup.
    /// </summary>
    public class ConfigurationBackup
    {
        public DateTime CreatedTime { get; set; }
        public string AdapterId { get; set; }
        public NetworkConfiguration Configuration { get; set; }
        public string ComputerName { get; set; }
        public string UserName { get; set; }
    }
}
