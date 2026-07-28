using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Core.Interfaces
{
    /// <summary>
    /// Represents context information for POS discovery.
    /// </summary>
    public class NetworkContext
    {
        public NetworkAdapter SelectedAdapter { get; set; }
        public List<ArpEntry> ArpEntries { get; set; }
        public DateTime ScanStartTime { get; set; }
    }

    /// <summary>
    /// Credentials for POS device authentication.
    /// </summary>
    public class PosCredentials
    {
        public string Username { get; set; }
        public string Password { get; set; }
        public string ApiKey { get; set; }
        public string SerialNumber { get; set; }
    }

    /// <summary>
    /// Interface for POS device discovery and configuration.
    /// </summary>
    public interface IPosConfigurationProvider
    {
        /// <summary>
        /// Friendly name of this provider (e.g., "Manual Configuration").
        /// </summary>
        string ProviderName { get; }

        /// <summary>
        /// Whether this provider supports device discovery.
        /// </summary>
        bool CanDiscover { get; }

        /// <summary>
        /// Whether this provider can read current device configuration.
        /// </summary>
        bool CanReadConfiguration { get; }

        /// <summary>
        /// Whether this provider can write/apply configuration to devices.
        /// </summary>
        bool CanWriteConfiguration { get; }

        /// <summary>
        /// Discover POS devices in the network.
        /// </summary>
        Task<IReadOnlyList<PosDevice>> DiscoverAsync(
            NetworkContext context,
            CancellationToken cancellationToken);

        /// <summary>
        /// Read current configuration from a POS device.
        /// </summary>
        Task<OperationResult<PosConfigurationData>> ReadConfigurationAsync(
            PosDevice device,
            PosCredentials credentials,
            CancellationToken cancellationToken);

        /// <summary>
        /// Apply configuration to a POS device.
        /// </summary>
        Task<OperationResult<PosConfigurationData>> ApplyConfigurationAsync(
            PosDevice device,
            NetworkConfiguration configuration,
            PosCredentials credentials,
            CancellationToken cancellationToken);

        /// <summary>
        /// Test connection to a POS device.
        /// </summary>
        Task<ConnectionTestResult> TestConnectionAsync(
            PosDevice device,
            CancellationToken cancellationToken);
    }

    /// <summary>
    /// POS configuration data returned by providers.
    /// </summary>
    public class PosConfigurationData
    {
        public NetworkConfiguration NetworkConfiguration { get; set; }
        public string ServiceIp { get; set; }
        public string DeviceType { get; set; }
        public string SerialNumber { get; set; }
        public string FirmwareVersion { get; set; }
        public string Model { get; set; }
    }
}
