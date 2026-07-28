using System;
using System.Collections.Generic;
using System.IO;
using ReHargi.PosNetworkManager.Core.Constants;

namespace ReHargi.PosNetworkManager.Infrastructure.Storage
{
    /// <summary>
    /// Service for saving and loading device profiles.
    /// Profiles are stored as JSON in ProgramData.
    /// </summary>
    public class ProfileStorage
    {
        private readonly string _profileDirectory;

        public ProfileStorage()
        {
            _profileDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData),
                AppConstants.StorageRoot,
                AppConstants.ProfilesSubfolder);

            EnsureDirectoryExists();
        }

        private void EnsureDirectoryExists()
        {
            if (!Directory.Exists(_profileDirectory))
                Directory.CreateDirectory(_profileDirectory);
        }

        /// <summary>
        /// Save a device profile.
        /// </summary>
        public bool SaveProfile(string profileName, DeviceProfile profile)
        {
            try
            {
                string filename = $"{CleanFilename(profileName)}.json";
                string filepath = Path.Combine(_profileDirectory, filename);

                // TODO: Serialize profile to JSON
                // For now, stub
                return true;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Load a device profile by name.
        /// </summary>
        public DeviceProfile LoadProfile(string profileName)
        {
            try
            {
                string filename = $"{CleanFilename(profileName)}.json";
                string filepath = Path.Combine(_profileDirectory, filename);

                if (!File.Exists(filepath))
                    return null;

                // TODO: Deserialize profile from JSON
                return null; // Stub
            }
            catch
            {
                return null;
            }
        }

        /// <summary>
        /// List all available profiles.
        /// </summary>
        public List<string> ListProfiles()
        {
            var profiles = new List<string>();
            try
            {
                foreach (var file in Directory.GetFiles(_profileDirectory, "*.json"))
                {
                    profiles.Add(Path.GetFileNameWithoutExtension(file));
                }
            }
            catch
            {
                // Silently fail
            }

            return profiles;
        }

        /// <summary>
        /// Delete a device profile.
        /// </summary>
        public bool DeleteProfile(string profileName)
        {
            try
            {
                string filename = $"{CleanFilename(profileName)}.json";
                string filepath = Path.Combine(_profileDirectory, filename);

                if (File.Exists(filepath))
                {
                    File.Delete(filepath);
                    return true;
                }
            }
            catch
            {
                // Silently fail
            }

            return false;
        }

        private string CleanFilename(string name)
        {
            return System.Text.RegularExpressions.Regex.Replace(name, @"[^a-zA-Z0-9_-]", "_");
        }
    }

    /// <summary>
    /// Represents a saved device profile.
    /// </summary>
    public class DeviceProfile
    {
        public string ProfileId { get; set; }
        public string ProfileName { get; set; }
        public string PcAdapterId { get; set; }
        public string PcPreferredIp { get; set; }
        public string PosPreferredIp { get; set; }
        public string PosMacAddress { get; set; }
        public string PosSerialNumber { get; set; }
        public string Gateway { get; set; }
        public string SubnetMask { get; set; }
        public string PrimaryDns { get; set; }
        public string SecondaryDns { get; set; }
        public string ProviderType { get; set; }
        public DateTime CreatedTime { get; set; }
        public DateTime LastVerifiedTime { get; set; }
        public string Notes { get; set; }
    }
}
