namespace ReHargi.PosNetworkManager.Core.Models
{
    /// <summary>
    /// Represents a network configuration (either for PC or POS).
    /// </summary>
    public class NetworkConfiguration
    {
        public string IpAddress { get; set; }
        public string SubnetMask { get; set; }
        public string Gateway { get; set; }
        public string PrimaryDns { get; set; }
        public string SecondaryDns { get; set; }
        public bool IsDhcpEnabled { get; set; }
    }
}
