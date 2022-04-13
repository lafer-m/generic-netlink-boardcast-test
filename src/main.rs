use std::error::Error;

// #[cfg(feature = "async")]
use neli::socket::tokio::NlSocket;
use neli::{
    consts::{
        nl::{GenlId, NlmF, NlmFFlags},
        socket::NlFamily, genl::{CtrlCmd, CtrlAttr},
    },
    genl::Genlmsghdr,
    nl::{NlPayload,Nlmsghdr},
    socket::{NlSocketHandle},
    types::GenlBuffer,
};
const GENL_NAME: &str = "linkagent";
const GENL_GROUP_NAME: &str = "linkagentgroup";


#[neli::neli_enum(serialized_type = "u8")]
pub enum LinkAgentCommand {
    Unspecified = 0,
    EchoMsg = 1,
    Register = 2,
}

impl neli::consts::genl::Cmd for LinkAgentCommand {}

#[neli::neli_enum(serialized_type = "u16")]
pub enum LinkAgentAttr {
    Unspecified = 0,
    EchoMsg = 1,
}

impl neli::consts::genl::NlAttrType for LinkAgentAttr {}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    // println!("Hello, world!");
    let mut sock = NlSocketHandle::connect(NlFamily::Generic, Some(0), &[])?;

    let family_id = sock.resolve_nl_mcast_group(GENL_NAME, GENL_GROUP_NAME)?;

    sock.add_mcast_membership(&[family_id])?;

    let mut buffer = Vec::new();
    
    let mut ss = NlSocket::new(sock)?;

    while let Ok(msgs) = ss.recv::<u16,Genlmsghdr<LinkAgentCommand,LinkAgentAttr>>(&mut buffer).await {
        // println!("msg: {:?}", msgs);
        for msg in msgs {
            // println!("receive from kernel: {:?}", msg);
            if let Ok(py) = msg.get_payload() {
                // for attr in py.get_attr_handle() {}
                let handle = py.get_attr_handle();
                if let Ok(msg) = handle.get_attr_payload_as_with_len::<String>(LinkAgentAttr::EchoMsg) {
                    println!("a msg from kernel: {}",msg);
                }
            }
        }
    }
    Ok(())
}
