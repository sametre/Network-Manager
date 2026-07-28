using System.Collections.Generic;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Core.Interfaces
{
    /// <summary>
    /// Interface for network adapter enumeration and configuration.
    /// </summary>
    public interface IAdapterService
    {
        /// <summary>
        /// Get all non-virtual, operational network adapters.
        /// </summary>
        Task<IReadOnlyList<NetworkAdapter>> GetActiveAdaptersAsync();

        /// <summary>
        /// Get adapter details by ID.
        /// </summary>
        Task<NetworkAdapter> GetAdapterByIdAsync(string adapterId);

        /// <summary>
        /// Read current configuration for a specific adapter.
        /// </summary>
        Task<NetworkConfiguration> ReadCurrentConfigurationAsync(string adapterId);

        /// <summary>
        /// Set static IP configuration for an adapter.
        /// </summary>
        Task<OperationResult> SetStaticConfigurationAsync(string adapterId, NetworkConfiguration config);

        /// <summary>
        /// Set adapter to DHCP mode.
        /// </summary>
        Task<OperationResult> SetDhcpConfigurationAsync(string adapterId);

        /// <summary>
        /// Restore a previously backed-up configuration.
        /// </summary>
        Task<OperationResult> RestoreConfigurationAsync(string adapterId, NetworkConfiguration config);
    }
}
