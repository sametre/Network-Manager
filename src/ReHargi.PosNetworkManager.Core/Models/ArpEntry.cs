namespace ReHargi.PosNetworkManager.Core.Models
{
    /// <summary>
    /// Represents an ARP table entry.
    /// </summary>
    public class ArpEntry
    {
        public string IpAddress { get; set; }
        public string MacAddress { get; set; }
        public ArpEntryType Type { get; set; }
        public string InterfaceName { get; set; }
        public int InterfaceIndex { get; set; }
    }

    /// <summary>
    /// ARP entry type enumeration.
    /// </summary>
    public enum ArpEntryType
    {
        Other = 1,
        Invalid = 2,
        Dynamic = 3,
        Static = 4
    }
}
