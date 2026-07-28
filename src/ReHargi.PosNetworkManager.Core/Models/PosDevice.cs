namespace ReHargi.PosNetworkManager.Core.Models
{
    /// <summary>
    /// Represents a POS device discovered on the network.
    /// </summary>
    public class PosDevice
    {
        public string Id { get; set; }
        public string IpAddress { get; set; }
        public string MacAddress { get; set; }
        public string Name { get; set; }
        public string Manufacturer { get; set; }
        public string Model { get; set; }
        public string SerialNumber { get; set; }
        public string FirmwareVersion { get; set; }
        public string ApplicationVersion { get; set; }
        public string DeviceType { get; set; }
        public int ResponseLatencyMs { get; set; }
        public double ConfidenceScore { get; set; }
        public IdentificationStatus IdentificationStatus { get; set; }
        public string ServiceIp { get; set; }
        public bool DhcpEnabled { get; set; }
        public string SubnetMask { get; set; }
        public string Gateway { get; set; }
        public string PrimaryDns { get; set; }
        public string SecondaryDns { get; set; }
        public System.DateTime LastSeenTime { get; set; }
    }

    /// <summary>
    /// Device identification status.
    /// </summary>
    public enum IdentificationStatus
    {
        Confirmed,
        Probable,
        Unknown,
        PreviouslyRegistered,
        ManualVerificationRequired
    }
}
