using System.Collections.Generic;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Core.Interfaces
{
    /// <summary>
    /// Interface for reading and managing ARP table.
    /// </summary>
    public interface IArpService
    {
        /// <summary>
        /// Get all ARP entries for a specific adapter.
        /// </summary>
        Task<IReadOnlyList<ArpEntry>> GetArpTableAsync(int interfaceIndex);

        /// <summary>
        /// Refresh ARP table entries.
        /// </summary>
        Task<OperationResult> RefreshArpTableAsync();

        /// <summary>
        /// Clear ARP entries for a specific IP on an adapter.
        /// </summary>
        Task<OperationResult> ClearArpEntryAsync(string ipAddress, int interfaceIndex);
    }
}
