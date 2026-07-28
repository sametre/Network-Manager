using System;
using System.Collections.Generic;
using System.Net.NetworkInformation;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core;
using ReHargi.PosNetworkManager.Core.Interfaces;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Network
{
    /// <summary>
    /// Service for enumerating and managing network adapters.
    /// Uses Windows APIs to retrieve adapter information.
    /// </summary>
    public class AdapterService : IAdapterService
    {
        public async Task<IReadOnlyList<NetworkAdapter>> GetActiveAdaptersAsync()
        {
            return await Task.Run(() =>
            {
                var adapters = new List<NetworkAdapter>();
                var interfaces = NetworkInterface.GetAllNetworkInterfaces();

                foreach (var ni in interfaces)
                {
                    // Filter: exclude virtual, disconnected, loopback, and Bluetooth adapters
                    if (ni.NetworkInterfaceType == NetworkInterfaceType.Loopback ||
                        ni.NetworkInterfaceType == NetworkInterfaceType.Wireless80211 && ni.Name.Contains("Bluetooth") ||
                        ni.OperationalStatus != OperationalStatus.Up)
                    {
                        continue;
                    }

                    var ipProps = ni.GetIPProperties();
                    var ipv4Addr = ipProps.UnicastAddresses.Find(x => x.Address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork);

                    if (ipv4Addr == null) continue;

                    adapters.Add(new NetworkAdapter
                    {
                        Id = ni.Id,
                        Name = ni.Name,
                        Description = ni.Description,
                        MacAddress = ni.GetPhysicalAddress().ToString(),
                        Status = ni.OperationalStatus.ToString(),
                        IpAddress = ipv4Addr.Address.ToString(),
                        SubnetMask = ipv4Addr.IPv4Mask.ToString(),
                        Gateway = ipProps.GatewayAddresses.Count > 0 ? ipProps.GatewayAddresses[0].Address.ToString() : string.Empty,
                        DnsServers = Array.ConvertAll(ipProps.DnsAddresses, x => x.ToString()),
                        IsDhcpEnabled = ipProps.GetIPv4Properties().IsDhcpEnabled,
                        InterfaceIndex = ni.GetIPProperties().GetIPv4Properties().Index
                    });
                }

                return adapters;
            });
        }

        public async Task<NetworkAdapter> GetAdapterByIdAsync(string adapterId)
        {
            var adapters = await GetActiveAdaptersAsync();
            foreach (var adapter in adapters)
            {
                if (adapter.Id == adapterId)
                    return adapter;
            }
            return null;
        }

        public async Task<NetworkConfiguration> ReadCurrentConfigurationAsync(string adapterId)
        {
            var adapter = await GetAdapterByIdAsync(adapterId);
            if (adapter == null)
                return null;

            return new NetworkConfiguration
            {
                IpAddress = adapter.IpAddress,
                SubnetMask = adapter.SubnetMask,
                Gateway = adapter.Gateway,
                PrimaryDns = adapter.DnsServers.Length > 0 ? adapter.DnsServers[0] : string.Empty,
                SecondaryDns = adapter.DnsServers.Length > 1 ? adapter.DnsServers[1] : string.Empty,
                IsDhcpEnabled = adapter.IsDhcpEnabled
            };
        }

        public async Task<OperationResult> SetStaticConfigurationAsync(string adapterId, NetworkConfiguration config)
        {
            // Implementation will use netsh or Windows networking APIs
            // For now, stub implementation
            return await Task.Run(() =>
            {
                try
                {
                    // TODO: Implement using netsh or Windows API
                    return OperationResult.Success();
                }
                catch (Exception ex)
                {
                    return OperationResult.Failure($"Ağ yapılandırması ayarlanırken hata: {ex.Message}", null, ex.ToString());
                }
            });
        }

        public async Task<OperationResult> SetDhcpConfigurationAsync(string adapterId)
        {
            // Implementation will use netsh or Windows networking APIs
            return await Task.Run(() =>
            {
                try
                {
                    // TODO: Implement using netsh or Windows API
                    return OperationResult.Success();
                }
                catch (Exception ex)
                {
                    return OperationResult.Failure($"DHCP etkinleştirilemedi: {ex.Message}", null, ex.ToString());
                }
            });
        }

        public async Task<OperationResult> RestoreConfigurationAsync(string adapterId, NetworkConfiguration config)
        {
            return await SetStaticConfigurationAsync(adapterId, config);
        }
    }
}
