// Placeholder models for Core layer

namespace ReHargi.PosNetworkManager.Core.Models
{
    /// <summary>
    /// Represents a network adapter on the system.
    /// </summary>
    public class NetworkAdapter
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string Status { get; set; }
        public string IpAddress { get; set; }
        public string SubnetMask { get; set; }
        public string Gateway { get; set; }
        public string[] DnsServers { get; set; }
        public string MacAddress { get; set; }
        public bool IsDhcpEnabled { get; set; }
        public int InterfaceIndex { get; set; }
        public NetworkCategory Category { get; set; }
    }

    /// <summary>
    /// Network category enumeration.
    /// </summary>
    public enum NetworkCategory
    {
        Public,
        Private,
        Domain
    }
}
