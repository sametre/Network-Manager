using System;
using System.Collections.Generic;

namespace ReHargi.PosNetworkManager.Core.Models
{
    /// <summary>
    /// Result of a connection test.
    /// </summary>
    public class ConnectionTestResult
    {
        public System.DateTime TestTime { get; set; }
        public List<IndividualTestResult> Results { get; set; } = new List<IndividualTestResult>();
        public bool IsSuccessful { get; set; }
        public string Summary { get; set; }
    }

    /// <summary>
    /// Result of an individual test within a connection test.
    /// </summary>
    public class IndividualTestResult
    {
        public string TestName { get; set; }
        public TestStatus Status { get; set; }
        public string Message { get; set; }
        public System.DateTime ExecutionTime { get; set; }
        public int DurationMs { get; set; }
    }

    /// <summary>
    /// Test status enumeration.
    /// </summary>
    public enum TestStatus
    {
        Pending,
        Running,
        Passed,
        Warning,
        Failed,
        Unsupported
    }
}
