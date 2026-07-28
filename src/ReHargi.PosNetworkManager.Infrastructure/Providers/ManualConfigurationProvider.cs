using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core;
using ReHargi.PosNetworkManager.Core.Interfaces;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Infrastructure.Providers
{
    /// <summary>
    /// Manual configuration provider for Stage 1.
    /// Produces instruction screens instead of automatic configuration.
    /// Does not report automatic success.
    /// </summary>
    public class ManualConfigurationProvider : IPosConfigurationProvider
    {
        public string ProviderName => "Manuel Yapılandırma";

        public bool CanDiscover => false;

        public bool CanReadConfiguration => false;

        public bool CanWriteConfiguration => false;

        public async Task<IReadOnlyList<PosDevice>> DiscoverAsync(
            NetworkContext context,
            CancellationToken cancellationToken)
        {
            await Task.Delay(0, cancellationToken);
            return new List<PosDevice>();
        }

        public async Task<OperationResult<PosConfigurationData>> ReadConfigurationAsync(
            PosDevice device,
            PosCredentials credentials,
            CancellationToken cancellationToken)
        {
            await Task.Delay(0, cancellationToken);
            return OperationResult<PosConfigurationData>.Failure(
                "Manuel yapılandırma sağlayıcısı cihaz bilgilerini okuyamaz.");
        }

        public async Task<OperationResult<PosConfigurationData>> ApplyConfigurationAsync(
            PosDevice device,
            NetworkConfiguration configuration,
            PosCredentials credentials,
            CancellationToken cancellationToken)
        {
            await Task.Delay(0, cancellationToken);

            // Manual provider does not modify device settings.
            // It returns an error indicating manual action is required.
            return OperationResult<PosConfigurationData>.Failure(
                "POS cihazı elle yapılandırılmalıdır. Aşağıdaki adımları izleyin.",
                null,
                BuildManualInstructions(device, configuration));
        }

        public async Task<ConnectionTestResult> TestConnectionAsync(
            PosDevice device,
            CancellationToken cancellationToken)
        {
            await Task.Delay(0, cancellationToken);

            return new ConnectionTestResult
            {
                TestTime = DateTime.Now,
                IsSuccessful = false,
                Summary = "Manuel sağlayıcı bağlantı testi desteklemez.",
                Results = new List<IndividualTestResult>()
            };
        }

        private string BuildManualInstructions(PosDevice device, NetworkConfiguration config)
        {
            var sb = new System.Text.StringBuilder();
            sb.AppendLine("=== POS Cihazı Manuel Yapılandırma Talimatları ===");
            sb.AppendLine();
            sb.AppendLine($"Cihaz IP: {device.IpAddress}");
            sb.AppendLine($"Cihaz MAC: {device.MacAddress}");
            sb.AppendLine();
            sb.AppendLine("Ağ Yönetim Menüsünde aşağıdaki ayarları yapın:");
            sb.AppendLine();
            sb.AppendLine("1. Ethernet Ağ Ayarları:");
            sb.AppendLine($"   - Mod: Statik");
            sb.AppendLine($"   - IP Adresi: {config.IpAddress}");
            sb.AppendLine($"   - Alt Ağ Maskesi: {config.SubnetMask}");
            sb.AppendLine($"   - Gateway: {config.Gateway}");
            sb.AppendLine($"   - DNS 1: {config.PrimaryDns}");
            if (!string.IsNullOrEmpty(config.SecondaryDns))
                sb.AppendLine($"   - DNS 2: {config.SecondaryDns}");
            sb.AppendLine();
            sb.AppendLine("2. Servis Ayarları:");
            sb.AppendLine($"   - Servis IP: 192.168.1.10");
            sb.AppendLine();

            return sb.ToString();
        }
    }
}
