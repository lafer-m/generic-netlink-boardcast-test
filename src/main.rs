use std::error::Error;
use std::fs::File;
#[macro_use] extern crate nix;
#[cfg(unix)]
use std::os::unix::io::{AsRawFd, RawFd};

use std::ffi::{CStr,CString};
use std::os::raw::c_void;

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
    open_dacs_d()?;
    println!("open the dacs_d");
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


const DACS_DEV_MAGIC: u8 = 'a' as u8;
const DACS_DEV_SET_IFF: u8 = 'a' as u8;
ioctl_write_ptr!(dacs_key_write, DACS_DEV_MAGIC, DACS_DEV_SET_IFF,  u64);


fn open_dacs_d() ->Result<(),Box<dyn Error>> {
    let write_str = Box::new("abcdefghikabcdefghikabcdefghikabcdefghikabcdefghikabcdefghikabcd".to_string());
    // let str_ptr = Box::into_raw(write_str);
    // let write_str = match write_str {
    //     Ok(s) => s,
    //     Err(e) => return Err(Box::new(e)),
    // };

    // println!("{:#?}", &write_str as *const u32)
          
    // println!("{:?}", &writeStr);
    // write_str.as_ptr()

    let f = File::open("/dev/dacs_d");
    let dacs_dev  = match f {
        Ok(d) => d,
        Err(e) => return Err(Box::new(e)),
    };

    
    let fd = dacs_dev.as_raw_fd();
    
    unsafe {
       
       let ret = dacs_key_write(fd, write_str.as_ptr() as *const u64);
       match ret {
           Ok(_) => return Ok(()),
           Err(e) => println!("call ioctl err:  {}", e),
       }
    };
    Ok(())
}
